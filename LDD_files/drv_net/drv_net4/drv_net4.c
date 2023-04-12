// For kernel module
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/module.h>

// For Network Device Driver
#include <linux/netdevice.h>	// For struct net_device 
#include <linux/skbuff.h>	// For struct sk_buff
#include <linux/etherdevice.h>	// For eth_type_trans()

// For accessing protocol headers
#include <linux/if_ether.h>	// For struct ethhdr and ETH_P_IP
#include <linux/ip.h>		// For struct iphdr 
#include <linux/udp.h>		// For struct udphdr 
#include <linux/tcp.h>		// For struct tcphdr 

// For accessing PHY Registers
#include <linux/socket.h>	// For SIOCGMIIPHY, SIOCGMIIREG etc
#include <linux/mii.h>		// For struct mii_ioctl_data and if_mii()

MODULE_LICENSE("GPL");

// Constant definitions
// These are the flags in the int_status
#define MAX_PHY_REG	10

// Macros for printing MAC address
#define MAC_FMT "%02x:%02x:%02x:%02x:%02x:%02x"
#define MAC_ARG(x) ((u8*)(x))[0],((u8*)(x))[1],((u8*)(x))[2],((u8*) (x))[3],((u8*)(x))[4],((u8*)(x))[5]

// Macro for printing IP address
#define NIPQUAD(addr) \
((unsigned char *)&addr)[0],\
((unsigned char *)&addr)[1],\
((unsigned char *)&addr)[2],\
((unsigned char *)&addr)[3]

// A structure representing an in-flight packet.
struct net_pkt_buffer
{
        struct net_pkt_buffer *next;
        struct net_device *dev;
        int datalen;
        unsigned char data[1518]; //Max length of Ethernet frame 14+1500+4 bytes
};

// Driver private data structure
struct net_dev_priv
{
    spinlock_t lock; 		// Spinlock to protect this structure 
    struct net_device *dev; // Save a pointer to corresponding struct net_device
    struct net_device_stats stats; // Save network statistics of this device
    struct sk_buff *skb;	// Save pointer to current sk_buff under process
    struct timer_list watchdog_timer;	// Timer for transmission timeout
    struct net_pkt_buffer *pbpool; //Singly-linked list (pool) of packet buffers
    int pbpool_size;		//Size of packet buffer pool
    struct net_pkt_buffer *rx_q; // Singly_linked list of incoming packets
    int phy_reg[MAX_PHY_REG];	// Simulate PHY Registers.
};

// Create two instances of Virtual Network Device (VND) 
struct net_device *drv_net_devs[2];

// Utility functions to print the network packet and its headers
// Print MAC header
static void drv_net_print_mac_hdr(struct ethhdr *eh)
{
    printk(KERN_DEBUG "---MAC Header---\n");
    printk(KERN_DEBUG "source mac:" MAC_FMT "\n", MAC_ARG(eh->h_source));
    printk(KERN_DEBUG "dest mac:" MAC_FMT "\n", MAC_ARG(eh->h_dest));
    printk(KERN_DEBUG "protocol: %04X\n", ntohs(eh->h_proto));
}

// Print IP header
static void drv_net_print_ip_hdr(struct iphdr *iph)
{
    printk(KERN_DEBUG "---IP Header---\n");
    printk(KERN_DEBUG "src: %u.%u.%u.%u, dst: %u.%u.%u.%u\n", 
           NIPQUAD(iph->saddr), NIPQUAD(iph->daddr));
    printk(KERN_DEBUG "protocol: %d\n", iph->protocol);
}

// Print UDP header
static void drv_net_print_udp_hdr(struct udphdr *uh)
{
    printk(KERN_DEBUG "---UDP Header---\n");
    printk(KERN_DEBUG "src_port: %d, dst_port: %d\n", 
           ntohs(uh->source), ntohs(uh->dest));
}

// Print TCP header
static void drv_net_print_tcp_hdr(struct tcphdr *th)
{
    printk(KERN_DEBUG "---TCP Header---\n");
    printk(KERN_DEBUG "src_port: %d, dst_port: %d\n", 
           ntohs(th->source), ntohs(th->dest));
}

// Print user/application data
static void drv_net_print_user_data(char *userdata)
{
    printk(KERN_DEBUG "---User Data---\n");
    // Assuming char data
    printk(KERN_DEBUG "%s\n", userdata);
}

// Print network packet 
static void drv_net_print_packet(struct sk_buff *skb)
{
    struct ethhdr *ethhdr = (struct ethhdr *)skb_mac_header(skb);
    struct iphdr *iphdr = (struct iphdr *)skb_network_header(skb);
    // Assuming UDP packet. Change it appropriately for TCP packets.
    struct udphdr *udphdr = (struct udphdr *)skb_transport_header(skb);
    // struct tcphdr *tcphdr = (struct tcphdr *)skb_transport_header(skb);

    printk(KERN_DEBUG "---Packet Dump---\n");
    printk(KERN_DEBUG "skb->len=%d\n", skb->len);

    if (skb->mac_header != 0)
        drv_net_print_mac_hdr(ethhdr);
    if (skb->network_header != 0)
        drv_net_print_ip_hdr(iphdr);
    if (skb->transport_header != 0)
    {
        // Assuming UDP packet. Change it appropriately for TCP packets.
        drv_net_print_udp_hdr(udphdr);
        // drv_net_print_tcp_hdr(tcphdr);
    }
    // Assuming UDP packet. Change it appropriately for TCP packets.
    drv_net_print_user_data((char *)(((char *)udphdr)+8));
}

// Set up device's packet buffer pool/queue.
static void drv_net_setup_pool(struct net_device *dev)
{
    struct net_dev_priv *nd_priv = netdev_priv(dev);
    struct net_pkt_buffer *pkt_buffer;
    int i;

    printk(KERN_INFO "drv_net_setup_pool() called\n");

    nd_priv->pbpool = NULL;
    for (i = 0; i < nd_priv->pbpool_size; i++)
    {
        pkt_buffer = kmalloc (sizeof (struct net_pkt_buffer), GFP_KERNEL);
        if (pkt_buffer == NULL)
        {
            printk (KERN_ERR "Ran out of memory allocating packet pool\n");
            return;
        }
        pkt_buffer->dev = dev;
        pkt_buffer->next = nd_priv->pbpool;
        nd_priv->pbpool = pkt_buffer;
    }
}

// Dismantle device's packet buffer pool/queue.
static void drv_net_teardown_pool(struct net_device *dev)
{
    struct net_dev_priv *nd_priv = netdev_priv(dev);
    struct net_pkt_buffer *pkt_buffer;

    printk(KERN_INFO "drv_net_teardown_pool() called\n");

    while (pkt_buffer = nd_priv->pbpool)
    {
        nd_priv->pbpool = pkt_buffer->next;
        kfree (pkt_buffer);
    }
}   

// Packet buffer pool/queue management functions
// Get a packet buffer from the packet buffer pool/queue.
static struct net_pkt_buffer *drv_net_get_pkt_buffer(struct net_device *dev)
{
    struct net_dev_priv *nd_priv = netdev_priv(dev);
    unsigned long flags;
    struct net_pkt_buffer *pkt_buffer;

    printk(KERN_INFO "drv_net_get_pkt_buffer() called\n");

    spin_lock_irqsave(&nd_priv->lock, flags);

    pkt_buffer = nd_priv->pbpool;
    nd_priv->pbpool = pkt_buffer->next;
    if (nd_priv->pbpool == NULL)
    {
        printk (KERN_INFO "Pool empty\n");
        netif_stop_queue(dev);
    }

    spin_unlock_irqrestore(&nd_priv->lock, flags);

    return pkt_buffer;
}

// Release a packet buffer to the packet buffer pool/queue.
static void drv_net_release_pkt_buffer(struct net_pkt_buffer *pkt_buffer)
{
    unsigned long flags;
    struct net_dev_priv *nd_priv = netdev_priv(pkt_buffer->dev);

    printk(KERN_INFO "drv_net_release_pkt_buffer() called\n");

    spin_lock_irqsave(&nd_priv->lock, flags);

    pkt_buffer->next = nd_priv->pbpool;
    nd_priv->pbpool = pkt_buffer;

    spin_unlock_irqrestore(&nd_priv->lock, flags);

    if (netif_queue_stopped(pkt_buffer->dev) && pkt_buffer->next == NULL)
        netif_wake_queue(pkt_buffer->dev);
}

// Add a packet buffer to Rx queue/list.
static void drv_net_enqueue_pkt_buffer(struct net_device *dev, struct net_pkt_buffer *pkt_buffer)
{
    unsigned long flags;
    struct net_dev_priv *nd_priv = netdev_priv(dev);

    printk(KERN_INFO "drv_net_enqueue_pkt_buffer() called\n");

    spin_lock_irqsave(&nd_priv->lock, flags);

    // Add the packet to the Rx queue/list.
    pkt_buffer->next = nd_priv->rx_q;
    nd_priv->rx_q = pkt_buffer;

    spin_unlock_irqrestore(&nd_priv->lock, flags);
}

// Remove a packet buffer from Rx queue/list.
static struct net_pkt_buffer *drv_net_dequeue_pkt_buffer(struct net_device *dev)
{
    struct net_dev_priv *nd_priv = netdev_priv(dev);
    struct net_pkt_buffer *pkt_buffer = nd_priv->rx_q;
    unsigned long flags;

    printk(KERN_INFO "drv_net_dequeue_pkt_buffer() called\n");

    spin_lock_irqsave(&nd_priv->lock, flags);

    // Remove the packet from the head of the Rx queue/list.
    if (pkt_buffer != NULL)
        nd_priv->rx_q = pkt_buffer->next;

    spin_unlock_irqrestore(&nd_priv->lock, flags);

    return pkt_buffer;
}

// Start network interface and start sending network packets.
static int drv_net_open(struct net_device *dev)
{
    printk(KERN_INFO "drv_net_open() called\n");

    /*
     *Request and assign mem region, I/O ports, irq etc in actual device driver.
     * Call request_region( ), request_irq( ) etc.
     *
     * Assign the hardware address of the board: use "\0CAPGx", where
     * x is 0 or 1. The first byte is ’\0’ to avoid being a multicast
     * MAC address (the first byte of multicast MAC address is odd).
     */

    // Fill up the MAC address of the first virtual network device: "\0CAPG0".
    memcpy(dev->dev_addr, "\0CAPG0", ETH_ALEN);	// MAC address: "\0CAPG0"

    // Fill up the MAC address of the second virtual network device: "\0CAPG1".
    if (dev == drv_net_devs[1] )
        dev->dev_addr[ETH_ALEN-1]++; // MAC address: "\0CAPG1"

    // Tell the network stack to start the packet transmission.
    netif_start_queue(dev);

    return 0;
}

// Stop network interface and stop sending network packets.
static int drv_net_stop(struct net_device *dev)
{
    printk(KERN_INFO "drv_net_stop() called\n");

    // Release Mem Region, I/O ports, irq and such in actual device driver.

    // Tell the network stack to stop the packet transmission.
    netif_stop_queue(dev);

    return 0;
}

// Receive incoming network packets and inject these packets into network stack.
static int drv_net_rx(struct net_device *dev, struct net_pkt_buffer *pkt_buffer)
{
    struct sk_buff *skb;
    struct net_dev_priv *nd_priv = netdev_priv(dev);

    printk(KERN_INFO "drv_net_rx() called\n");

    /*
     * In actual device drivers, the network packets are retrieved from the
     * DMA buffer at this point in code.
     * Build an skb around it, so upper layers can handle it. Add 2 bytes 
     * of buffer space for IP header alignment.
     */
    skb = dev_alloc_skb(pkt_buffer->datalen + 2);
    if (!skb)
    {
        printk(KERN_NOTICE "drv_net_rx(): No memory - packet dropped\n");
        nd_priv->stats.rx_dropped++;
        goto err_alloc_skb;
    }
    
    // Copy the packet data from packet buffer to newly allocated SKB.
    memcpy(skb_put(skb, pkt_buffer->datalen), pkt_buffer->data, pkt_buffer->datalen);

    // Write some metadata into SKB.
    skb->dev = dev;
    skb->protocol = eth_type_trans(skb, dev);
    skb->ip_summed = CHECKSUM_UNNECESSARY; // Don’t check checksum.

    // Update the network statistics.
    nd_priv->stats.rx_packets++;
    nd_priv->stats.rx_bytes += pkt_buffer->datalen;

    // Pass the network packet (SKB) to the network stack. 
    netif_rx(skb);

    return 0;

err_alloc_skb:
    return -1;
}

/*
 * Transmit outgoing network packets - handle actual device HW dependent code.
 * This function deals with hw details in real device driver.
 * In this case, this virtual network device/interface loops back
 * the packet to the other virtual network device/interface (if any).
 * In other words, this function implements the virtual network device
 * behaviour, while all other fucntions are rather device-independent.
 */
static void drv_net_hw_tx(char *data, int len, struct net_device *txdev)
{
    struct iphdr *ih;
    struct net_device *rxdev;
    struct net_dev_priv *nd_priv;
    unsigned int *saddr, *daddr;
    struct net_pkt_buffer *pkt_buffer;

    printk(KERN_INFO "drv_net_hw_tx() called\n");

    if (len < sizeof(struct ethhdr) + sizeof(struct iphdr)) 
    {
        printk("drv_net: packet too short (%d bytes)\n", len);
        return;
    }

    // Enable this conditional "if" to print the packet data.
    if (0) 
    {
        int i;
        printk(KERN_DEBUG "len is %d\n" KERN_DEBUG "data:",len);
        for (i=14 ; i<len; i++)
            printk(" %02x",data[i]&0xff);
        printk("\n");
    }
    /*
     * Ethernet/MAC header is 14 bytes, but the network stack arranges for
     * IP header to be aligned at 4-byte boundary (i.e. Ethernet/MAC 
     * header is unaligned).
     */
    ih = (struct iphdr *)(data+sizeof(struct ethhdr));
    saddr = &ih->saddr;
    daddr = &ih->daddr;

    /*
     * Manipulate the IP header to pretend that the packet is coming from
     * another host from another network. 
     * Change the third byte of class C source and destination IP addresses,
     * so that the network address part of the IP address gets changed.
     */
    ((unsigned char *)saddr)[2] ^= 1;
    ((unsigned char *)daddr)[2] ^= 1;

    // Rebuild the checksum. IP needs it.
    ih->check = 0;
    ih->check = ip_fast_csum((unsigned char *)ih,ih->ihl);

    // Now the packet is ready for transmission.
    rxdev = drv_net_devs[txdev == drv_net_devs[0] ? 1 : 0];
    nd_priv = netdev_priv(rxdev);

    // Pick-up a packet buffer from the packet buffer pool/queue.
    pkt_buffer = drv_net_get_pkt_buffer(txdev);
    pkt_buffer->datalen = len;

    // Copy the data to the packet buffer.  
    memcpy(pkt_buffer->data, data, len);

/*
 * This is the point where the network packet is changing the direction
 * from TX path to RX path and from/to vnd0 N/W iface to/from vnd1 N/W iface.
 *             TX ===============> RX
 *            vnd0 <============> vnd1
 * drv_net_devs[0] <============> drv_net_devs[1] 
 */

    // Add the packet buffer to the Rx queue/list.
    drv_net_enqueue_pkt_buffer(rxdev, pkt_buffer);

    /*
     * At this point, incoming packet is supposed to have been received
     * in the packet buffer. Remove a packet from Rx queue for processing.
     */
    pkt_buffer = drv_net_dequeue_pkt_buffer(rxdev);
    if (pkt_buffer)
    {
        // Send the incoming packet to drv_net_rx() for handling.
        drv_net_rx(rxdev, pkt_buffer);

        /*
         * At this point,the incoming packet has been sent to the network stack.
         * Now release the packet buffer back to the packet buffer pool.
         */
        drv_net_release_pkt_buffer(pkt_buffer);
    }

    /*
     * At this point, outgoing packet is supposed to have been
     * transmitted. Update the network statistics and free the SKB.
     */
    nd_priv = netdev_priv(txdev);
    nd_priv->stats.tx_packets++;
    nd_priv->stats.tx_bytes += len;

    // Free up SKB.
    if (nd_priv->skb)
    {
        dev_kfree_skb(nd_priv->skb);
        nd_priv->skb = NULL;
    }
}

// Transmit outgoing network packets.
static netdev_tx_t drv_net_tx(struct sk_buff *skb, struct net_device *dev)
{
    int len;
    char *data, shortpkt[ETH_ZLEN];
    struct net_dev_priv *nd_priv = netdev_priv(dev);

    printk(KERN_INFO "drv_net_tx() called\n");

    // Print the packet content, if required.
    drv_net_print_packet(skb);

    /*
     * If packet is shorter than the minimum length, then copy the data into
     * a short packet buffer before processing.
     */
    data = skb->data;
    len = skb->len;
    if (len < ETH_ZLEN)
    {
        memset(shortpkt, 0, ETH_ZLEN);
        memcpy(shortpkt, skb->data, skb->len);
        len = ETH_ZLEN;
        data = shortpkt;
    }

    // Save the timestamp of packet transmission.
    dev->_tx->trans_start = jiffies;

    // Remember the skb, so we can free it after transmitting the packet.
    nd_priv->skb = skb;

    /* 
     * Actual transmission of data is device-specific.
     * Perform device-specific transmission in the called fucntion below. 
     */
    drv_net_hw_tx(data, len, dev);

    return NETDEV_TX_OK;
}

/*
 * IOCTL: As there is no real hardware, emulate PHY registers in driver private
 * data.
 */
static int drv_net_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
    struct mii_ioctl_data *data = if_mii(ifr);
    struct net_dev_priv *nd_priv = netdev_priv(dev);

    printk(KERN_INFO "drv_net_ioctl(): cmd = 0X%X\n", cmd);

    switch (cmd)
    {
    case SIOCGMIIPHY:
        printk(KERN_INFO "drv_net_ioctl(): SIOCGMIIPHY\n");
        data->phy_id = (dev->dev_addr[5]) & 0x1;
        break;

    case SIOCGMIIREG:
        if (data->reg_num >= MAX_PHY_REG)
        {
            printk(KERN_WARNING "drv_net_ioctl(): SIOCGMIIREG, reg=%d not supported\n", data->reg_num);
            return -1;
        }
        data->val_out = nd_priv->phy_reg[data->reg_num];
        break;

    case SIOCSMIIREG:
        if (data->reg_num >= MAX_PHY_REG)
        {
            printk(KERN_WARNING "drv_net_ioctl(): SIOCSMIIREG, reg=%d not supported\n", data->reg_num);
            return -1;
        }
        nd_priv->phy_reg[data->reg_num] = data->val_in;
        break;

    default:
        printk(KERN_ERR "drv_net_ioctl(): cmd = 0X%X not supported\n", cmd);
        return -1;
    }

    return 0;
}

// Fetch the network statistics of the given network device.
static struct net_device_stats *drv_net_stats(struct net_device *dev)
{
    struct net_dev_priv *nd_priv = netdev_priv(dev);

    printk(KERN_INFO "drv_net_stats() called\n");

    return &nd_priv->stats;
}


// Function for chaning the maximum transmission unit (MTU) -> max packet size.
static int drv_net_change_mtu(struct net_device *dev, int new_mtu)
{
    unsigned long flags;
    struct net_dev_priv *priv = netdev_priv(dev);

    printk(KERN_INFO "drv_net_change_mtu() called\n");

    // Check the ranges.
    if ((new_mtu < 68) || (new_mtu > 1500))
        return -EINVAL;

    // Do anything you need here, and then accept the value.

    spin_lock_irqsave(&priv->lock, flags);
    dev->mtu = new_mtu;
    spin_unlock_irqrestore(&priv->lock, flags);

    return 0;
}

// Deal with a transmission timeout.
static void drv_net_tx_timeout (struct timer_list *t)
{
    struct net_dev_priv *nd_priv = from_timer(nd_priv, t, watchdog_timer);
    struct net_device *dev = nd_priv->dev;

    printk(KERN_INFO "drv_net_tx_timeout() called\n");

    printk(KERN_DEBUG "Transmit timeout at %ld, latency %ld\n", jiffies,
            jiffies - dev->_tx->trans_start);

    // Update the network statistics.
    nd_priv->stats.tx_errors++;

    // Free up SKB.
    if (nd_priv->skb)
    {
        dev_kfree_skb(nd_priv->skb);
        nd_priv->skb = NULL;
    }

    // Tell the network stack to start sending packet (SKB) again.
    netif_wake_queue(dev);

    return;
}

// Function to allow device driver (instead of ARP) to create MAC header.
static int drv_net_header(struct sk_buff *skb, struct net_device *dev,
                          unsigned short type, const void *daddr,
                          const void *saddr,unsigned int len)
{
    struct ethhdr *eth = (struct ethhdr *)skb_push(skb,ETH_HLEN);

    printk(KERN_INFO "drv_net_header() called\n");

    eth->h_proto = htons(type);
    memcpy(eth->h_source, saddr ? saddr : dev->dev_addr, dev->addr_len);
    memcpy(eth->h_dest, daddr ? daddr : dev->dev_addr, dev->addr_len);

    // Flip the LSBit of destination MAc address (of the other interface). 
    eth->h_dest[ETH_ALEN-1] ^= 0x01;

    return (dev->hard_header_len);
}

static struct net_device_ops nd_ops =
{
    .ndo_open = drv_net_open,
    .ndo_stop = drv_net_stop,
    .ndo_start_xmit = drv_net_tx,
    .ndo_do_ioctl = drv_net_ioctl,
    .ndo_change_mtu = drv_net_change_mtu,
    .ndo_get_stats = drv_net_stats
};

static struct header_ops hdr_ops =
{
    .create = drv_net_header    
}; 

// Network device setup/initialization function.
static void drv_net_setup(struct net_device *dev)
{
    struct net_dev_priv *nd_priv = (struct net_dev_priv *)netdev_priv(dev);

    printk(KERN_INFO "drv_net_setup()\n");

    ether_setup(dev);
    dev->netdev_ops = &nd_ops;
    dev->header_ops = &hdr_ops;

    /*
     * keep the default flags, just add NOARP as we can not use ARP
     * with this virtual device driver.
     */
    dev->flags |= IFF_NOARP;
    // Pretend that checksum is performed in the hardware.
    //dev->features |= NETIF_F_HW_CSUM; // Not required

    // Initialize nd_priv with zeros.
    memset(nd_priv, 0, sizeof(struct net_dev_priv));

    //Save pointer to struct net_device in corresponding struct net_dev_priv
    nd_priv->dev = dev;

    dev->watchdog_timeo = 5*HZ; // 5 seconds packet Tx timeout
    // Set-up Tx timeout timer.
    timer_setup(&nd_priv->watchdog_timer, drv_net_tx_timeout, 0);

    // Initialize spinlock to protect access to struct net_dev_priv.
    spin_lock_init(&nd_priv->lock);

    // Set-up packet buffer pool/queue.
    nd_priv->pbpool_size = 8;
    drv_net_setup_pool(dev);

    // Perform additional initialization - hardware initialization etc here.

    return;
}

static int __init drv_net_init(void)
{
    int i;
    int result;
    struct net_device *dev;
    struct net_dev_priv *nd_priv;

    printk(KERN_INFO "Hello Kernel\n");

    /* 
     * alloc_netdev() allocates memory for struct net_dev_priv
     * along with struct net_device.
     */
    dev = alloc_netdev(sizeof(struct net_dev_priv), "vnd%d", NET_NAME_UNKNOWN,
                       drv_net_setup);
    if (dev == NULL)
    {
        printk(KERN_ERR "Failed to allocate netdev for dev 0\n");
        return -1;
    }
    drv_net_devs[0] = dev;

    dev = alloc_netdev(sizeof(struct net_dev_priv), "vnd%d", NET_NAME_UNKNOWN,
                       drv_net_setup);
    if (dev == NULL)
    {
        printk(KERN_ERR "Failed to allocate netdev for dev 1\n");
        dev = drv_net_devs[0];
        nd_priv = (struct net_dev_priv *)netdev_priv(dev);

        drv_net_teardown_pool(dev);
        del_timer_sync(&nd_priv->watchdog_timer);
        free_netdev(dev);

        return -1;
    }
    drv_net_devs[1] = dev;

    for (i = 0; i < 2; i++)
    {
        dev = drv_net_devs[i];
        // Register the network device.
        if ((result = register_netdev(dev)))
        {
            printk("drv_net: error %d registering device %s",result,
                   drv_net_devs[i]->name);
            if (i == 1)
            {
                unregister_netdev(drv_net_devs[0]);
            }
            goto err_reg_netdev;
        }
        
    }

    return 0;
 
err_reg_netdev:
    for (i = 0; i < 2; i++)
    {
        dev = drv_net_devs[i];
        nd_priv = (struct net_dev_priv *)netdev_priv(dev);

        drv_net_teardown_pool(dev);
        del_timer_sync(&nd_priv->watchdog_timer);
        free_netdev(dev);
    }

    return -1;
}

static void __exit drv_net_exit(void)
{
    int i;
    struct net_device *dev;
    struct net_dev_priv *nd_priv;

    for (i = 0; i < 2; i++)
    {
        dev = drv_net_devs[i];
        nd_priv = (struct net_dev_priv *)netdev_priv(dev);
        if (dev)
        {
            unregister_netdev(dev);
            drv_net_teardown_pool(dev);
            del_timer_sync(&nd_priv->watchdog_timer);
            free_netdev(dev);
        }
    }

    printk(KERN_INFO "Bye-bye Kernel\n");
    return ;
}
 
module_init(drv_net_init);
module_exit(drv_net_exit);

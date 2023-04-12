#include <stdio.h>

int add(int a, int b)
{
    return a+b;
}


int subtract(int a, int b)
{
    return a-b;
}

int multiply(int a, int b)
{
    return a*b;

}

int divide(int a, int b)
{
    return a/b;
}

void main(void)
{
    char c;
    int result;
    int a, b;

    	printf("Press 'a' for addition.\n");
    	printf("Press 's' for subtraction.\n");
    	printf("Press 'm' for multiplication.\n");
    	printf("Press 'd' for division.\n");
    	printf("Press 'x' to exit.\n");
    	printf("Enter your choice:");
	scanf("%c", &c);

    	printf("Enter first number: ");
    	scanf("%d", &a);
    	printf("Enter second number: ");
    	scanf("%d", &b);

    	switch(c)
    	{
    	case 'a':
	    result = add(a, b);
	    printf("addition=%d\n", result);
    	    break;

    	case 's':
	    result = subtract(a, b);
	    printf("subtraction=%d\n", result);
    	    break;

    	case 'm':
	    result = multiply(a, b);
	    printf("multiplication=%d\n", result);
    	    break;

    	case 'd':
	    result = divide(a, b);
	    printf("division=%d\n", result);
    	    break;

    	case 'x':
    	    break;

    	default:
	    printf("Invalid option\n");
    	    break;
    	}

    return;
}

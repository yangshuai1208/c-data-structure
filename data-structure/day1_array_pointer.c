#include <assert.h>
#include <stddef.h>
#include <stdio.h>

static int  array_sum(const int *arr,size_t length)
{
    if(arr==NULL)
    {
        return 0;
    }
    int sum=0;

    for(size_t i=0;i<length;i++)
    {
        sum+=arr[i];
    }
    return sum;
}

static void array_reverse(int *arr,size_t length)
{
    if(arr==NULL||length<2U)
    {
        return;
    }
    size_t left=0U;
    size_t right=length-1U;

    while(left<right)
    {
        int temporary=arr[left];
        arr[left]=arr[right];
        arr[right]=temporary;

        ++left;
        right--;
    }
}
static void print_array(const int *arr,size_t length)
{
    if(arr==NULL)
    {
        return;
    }
    for(size_t i=0U;i<length;++i)
    {
        printf("%d ",arr[i]);
    }
    printf("\n");
}
int main()
{
    int data[]={1,2,3,4,5};

    size_t length=sizeof(data)/sizeof(data[0]);
    int *pointer=data;

    printf("array bytes  =%zu\n",sizeof(data));
    printf("pointer bytes=%zu\n",sizeof(pointer));
    printf(" length =%zu\n",length);

    assert(data[2]==*(data+2));
    assert(pointer[2]==*(pointer+2));
    assert(array_sum(data,length)==15);

    array_reverse(data,length);

    print_array(data,length);

        assert(data[0]==5);
        assert(data[1]==4);
        assert(data[2]==3);
        assert(data[3]==2);
        assert(data[4]==1);

        puts("array pointer test passed");
        return 0;
}
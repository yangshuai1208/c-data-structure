#include <assert.h>
#include <stddef.h>
#include <stdio.h>

typedef struct ListNode
{
    int value;
    struct ListNode *next;
} ListNode;

static ListNode *merge_sorted_lists(
    ListNode *list1,
    ListNode *list2
)
{
    ListNode dummy = {0, NULL};
    ListNode *tail = &dummy;

    while (list1 != NULL && list2 != NULL)
    {
        if (list1->value <= list2->value)
        {
            tail->next = list1;
            list1 = list1->next;
        }
        else
        {
            tail->next = list2;
            list2 = list2->next;
        }

        tail = tail->next;
    }

    if (list1 != NULL)
    {
        tail->next = list1;
    }
    else
    {
        tail->next = list2;
    }

    return dummy.next;
}

static void test_normal_lists(void)
{
    ListNode node5 = {5, NULL};
    ListNode node3 = {3, &node5};
    ListNode node1 = {1, &node3};

    ListNode node6 = {6, NULL};
    ListNode node4 = {4, &node6};
    ListNode node2 = {2, &node4};

    ListNode *head = merge_sorted_lists(&node1, &node2);

    assert(head == &node1);
    assert(head->value == 1);

    assert(head->next == &node2);
    assert(node2.next == &node3);
    assert(node3.next == &node4);
    assert(node4.next == &node5);
    assert(node5.next == &node6);
    assert(node6.next == NULL);
}

static void test_empty_lists(void)
{
    ListNode node = {10, NULL};

    assert(merge_sorted_lists(NULL, NULL) == NULL);
    assert(merge_sorted_lists(NULL, &node) == &node);
}

int main(void)
{
    test_normal_lists();
    test_empty_lists();

    printf("merge sorted lists tests passed\n");
    return 0;
}
#include <assert.h>
#include <stddef.h>
#include <stdio.h>

typedef struct ListNode
{
    int value;
    struct ListNode *next;
} ListNode;

static ListNode *reverse_list(ListNode *head)
{
    ListNode *prev = NULL;
    ListNode *current = head;

    while (current != NULL)
    {
        ListNode *next =current->next; /* 1. 保存下一个节点 */

        current->next = prev;/* 2. 反转指针 */

        /* 3. prev向前移动 */
        prev =current; /* 填写 */

        /* 4. current向前移动 */
        current =next; /* 填写 */;
    }

    /* 新的头节点 */
    return prev;/* 填写 */;
}

static void test_normal_list(void)
{
    ListNode node4 = {4, NULL};
    ListNode node3 = {3, &node4};
    ListNode node2 = {2, &node3};
    ListNode node1 = {1, &node2};

    ListNode *head = reverse_list(&node1);

    assert(head == &node4);
    assert(head->value == 4);
    assert(head->next == &node3);
    assert(node3.next == &node2);
    assert(node2.next == &node1);
    assert(node1.next == NULL);
}

static void test_empty_list(void)
{
    assert(reverse_list(NULL) == NULL);
}

static void test_single_node(void)
{
    ListNode node = {10, NULL};

    ListNode *head = reverse_list(&node);

    assert(head == &node);
    assert(head->value == 10);
    assert(head->next == NULL);
}

int main(void)
{
    test_normal_list();
    test_empty_list();
    test_single_node();

    printf("reverse linked list tests passed\n");
    return 0;
}
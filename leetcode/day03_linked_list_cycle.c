#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct ListNode
{
    int value;
    struct ListNode *next;
} ListNode;

static bool has_cycle(const ListNode *head)
{
    const ListNode *slow = head;
    const ListNode *fast = head;

    while (fast != NULL && fast->next != NULL)
    {
        slow = slow->next;
        fast = fast->next->next;

        if (slow == fast)
        {
            return true;
        }
    }

    return false;
}

int main(void)
{
    ListNode node1 = {1, NULL};
    ListNode node2 = {2, NULL};
    ListNode node3 = {3, NULL};
    ListNode node4 = {4, NULL};
    ListNode self = {5, NULL};

    /* 测试1：空链表 */
    assert(has_cycle(NULL) == false);

    /* 测试2：单节点无环 */
    assert(has_cycle(&self) == false);

    /* 测试3：单节点自身成环 */
    self.next = &self;
    assert(has_cycle(&self) == true);
    self.next = NULL;

    /* 构造普通链表：1 -> 2 -> 3 -> 4 -> NULL */
    node1.next = &node2;
    node2.next = &node3;
    node3.next = &node4;
    node4.next = NULL;

    /* 测试4：多节点无环 */
    assert(has_cycle(&node1) == false);

    /* 构造环：node4 -> node2 */
    node4.next = &node2;

    /* 测试5：多节点有环 */
    assert(has_cycle(&node1) == true);

    printf("linked list cycle tests passed\n");
    return 0;
}
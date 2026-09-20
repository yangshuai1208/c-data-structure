#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

void set_bit(uint32_t *reg, uint8_t bit)
{
    if (reg == NULL || bit >= 32U)
    {
        return;
    }
    *reg |=(1U<<bit);
}
void clear_bit(uint32_t *reg, uint8_t bit)
{
    if (reg == NULL || bit >= 32U)
    {
        return;
    }
    *reg &=~(1U<<bit);
}
void toggle_bit(uint32_t *reg, uint8_t bit)
{
    if (reg == NULL || bit >= 32U)
    {
        return;
    }
    *reg^=(1U<<bit);
}
bool get_bit(uint32_t reg, uint8_t bit)
{
    if (bit >= 32U)
    {
        return false;
    }
    return (reg&(1U<<bit))!=0;
}
int main(void)
{
    uint32_t reg = 0;

    /* bit 3置1 */
    set_bit(&reg, 3);
    assert(reg == 0x00000008U);
    assert(get_bit(reg, 3) == true);

    /* bit 1置1 */
    set_bit(&reg, 1);
    assert(reg == 0x0000000AU);

    /* bit 3清0 */
    clear_bit(&reg, 3);
    assert(reg == 0x00000002U);

    /* bit 1翻转 */
    toggle_bit(&reg, 1);
    assert(reg == 0x00000000U);

    /* bit 31测试 */
    set_bit(&reg, 31);
    assert(reg == 0x80000000U);


    uint32_t old = reg;

    set_bit(&reg, 32);

    assert(reg == old);
    assert(get_bit(reg, 32) == false);

    printf("All tests passed!\n");

    return 0;
}
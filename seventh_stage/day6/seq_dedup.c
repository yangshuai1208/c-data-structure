#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct
{
    bool valid;
    uint32_t last_seq;
} seq_dedup_t;

void seq_dedup_init(seq_dedup_t *ctx)
{
    if(ctx==NULL)
    {
        return;
    }
    ctx->valid=false;
    ctx->last_seq=0;
}

bool seq_dedup_is_duplicate(
    seq_dedup_t *ctx,
    uint32_t seq)
{
   
    if(ctx==NULL)
    {
        return false;
    }
    if(!ctx->valid)
    {
        ctx->last_seq=seq;
        ctx->valid=true;
        return false;  
    }
    if(seq==ctx->last_seq)
    {
        return true;
    }
    ctx->last_seq=seq;
    return false;

}

int main(void)
{
    seq_dedup_t ctx;

    seq_dedup_init(&ctx);

    assert(
        seq_dedup_is_duplicate(
            &ctx, 10) == false);

    assert(
        seq_dedup_is_duplicate(
            &ctx, 10) == true);

    assert(
        seq_dedup_is_duplicate(
            &ctx, 11) == false);

    assert(
        seq_dedup_is_duplicate(
            &ctx, 11) == true);

    printf("seq_dedup passed\n");

    return 0;
}
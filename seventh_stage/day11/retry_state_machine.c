#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

typedef enum
{
    REQ_IDLE,
    REQ_WAIT_ACK,
    REQ_EXECUTING,
    REQ_SUCCEEDED,
    REQ_FAILED
} RequestState;

typedef struct
{
    uint32_t seq;
    RequestState state;

    uint32_t last_send_ms;
    uint32_t timeout_ms;

    unsigned int retry_count;
    unsigned int max_retries;
} Request;


void request_init(
    Request *req,
    uint32_t seq,
    uint32_t now_ms)
{
    if (req == NULL)
    {
        return;
    }

    req->seq = seq;
    req->state = REQ_WAIT_ACK;

    req->last_send_ms = now_ms;


    req->timeout_ms = 1500U;

    req->retry_count = 0U;


    req->max_retries = 2U;
}


bool request_should_retry(
    Request *req,
    uint32_t now_ms)
{
    if (req == NULL)
    {
        return false;
    }


    if (req->state == REQ_SUCCEEDED ||
        req->state == REQ_FAILED ||
        req->state == REQ_IDLE)
    {
        return false;
    }


    uint32_t elapsed =
        (uint32_t)(now_ms - req->last_send_ms);


    if (elapsed < req->timeout_ms)
    {
        return false;
    }


    if (req->retry_count >= req->max_retries)
    {
        req->state = REQ_FAILED;
        return false;
    }


    req->retry_count++;


    req->last_send_ms = now_ms;

    return true;
}


void request_on_ack_in_progress(
    Request *req,
    uint32_t now_ms)
{
    if (req == NULL)
    {
        return;
    }


    if (req->state == REQ_SUCCEEDED ||
        req->state == REQ_FAILED)
    {
        return;
    }

    req->state = REQ_EXECUTING;

    req->timeout_ms = 5000U;


    req->last_send_ms = now_ms;
}


void request_on_ack_ok(
    Request *req)
{
    if (req == NULL)
    {
        return;
    }

    if (req->state == REQ_SUCCEEDED ||
        req->state == REQ_FAILED)
    {
        return;
    }

    req->state = REQ_SUCCEEDED;
}

int main(void)
{
    Request req;


    request_init(&req, 100U, 1000U);

    assert(req.seq == 100U);
    assert(req.state == REQ_WAIT_ACK);
    assert(req.last_send_ms == 1000U);
    assert(req.timeout_ms == 1500U);
    assert(req.retry_count == 0U);
    assert(req.max_retries == 2U);


    assert(!request_should_retry(
        &req,
        2000U));

    assert(req.retry_count == 0U);



    assert(request_should_retry(
        &req,
        2500U));

    assert(req.retry_count == 1U);
    assert(req.last_send_ms == 2500U);



    assert(request_should_retry(
        &req,
        4000U));

    assert(req.retry_count == 2U);
    assert(req.last_send_ms == 4000U);


    assert(!request_should_retry(
        &req,
        5500U));

    assert(req.state == REQ_FAILED);


    assert(!request_should_retry(
        &req,
        10000U));



    request_init(&req, 101U, 1000U);

    request_on_ack_in_progress(
        &req,
        1200U);

    assert(req.state == REQ_EXECUTING);
    assert(req.timeout_ms == 5000U);
    assert(req.last_send_ms == 1200U);


    assert(!request_should_retry(
        &req,
        6199U));


    request_on_ack_ok(&req);

    assert(req.state == REQ_SUCCEEDED);

    assert(!request_should_retry(
        &req,
        10000U));


    request_init(
        &req,
        102U,
        UINT32_MAX - 100U);

    req.timeout_ms = 200U;


    assert(!request_should_retry(
        &req,
        50U));

    assert(request_should_retry(
        &req,
        150U));



    request_init(NULL, 1U, 0U);

    assert(!request_should_retry(
        NULL,
        1000U));

    request_on_ack_in_progress(
        NULL,
        1000U);

    request_on_ack_ok(NULL);

    printf("retry_state_machine passed\n");

    return 0;
}
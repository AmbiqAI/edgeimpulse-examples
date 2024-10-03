
/**
 *  \file BT_timer.c
 *
 *  This File contains source codes for the EtherMind
 *  Timer Library Implementation for Windows (User-mode).
 */

/*
 *  Copyright (C) 2013. Mindtree Ltd.
 *  All rights reserved.
 */

/* ----------------------------------------------- Header File Inclusion */
#include "BT_timer_internal.h"


/* ----------------------------------------------- Global Definitions */
/* Timer Elements */
TIMER_ENTITY timer_entity[TIMER_MAX_ENTITIES];
TIMER_ENTITY *timer_q_start = NULL;
TIMER_ENTITY *timer_q_end   = NULL;

/* Timer Thread/Task ID */
BT_thread_type timer_tid;

/* Timer Library Mutex */
BT_DEFINE_MUTEX (timer_mutex)

/* Timer Library Condition Variable & Mutex */
BT_DEFINE_COND (timer_cv)
BT_DEFINE_MUTEX (timer_cv_mutex)

/* Timer State */
UCHAR timer_state = TIMER_INIT;

portTickType current_tick;
xTimerHandle base_timer_handle;

/* ----------------------------------------------- Static Global Variables */


/* ----------------------------------------------- Functions */

void base_timer_cb (void * timer_id)
{
    /* Lock Timer */
    timer_lock();

    current_tick++;

    timer_signal();

     /* Lock Timer */
    timer_unlock();
}

void em_timer_init (void)
{
    UINT16 index;

    BT_TIMER_TRC(
    "Initializing EtherMind Timer Library Module ...\n");

    /* Initialize Timer Mutex */
    BT_MUTEX_INIT_VOID (timer_mutex, TIMER);

    /* Initialize Timer State */
    timer_state = TIMER_INIT;

    /* Initialize Timer Conditional Variables */
    BT_COND_INIT_VOID (timer_cv, TIMER);
    BT_MUTEX_INIT_VOID (timer_cv_mutex, TIMER);

    /* Initialize Timer Elements */
    for (index = 0; index < TIMER_MAX_ENTITIES; index ++)
    {
        timer_init_entity(&timer_entity[index]);
    }

    current_tick = 0;

    /* Create Timer Thread */
    BT_TIMER_TRC(
    "Creating Timer Task ...\n");
    base_timer_handle = xTimerCreate ("Base Timer",1000/portTICK_RATE_MS, pdTRUE,&current_tick,base_timer_cb);
    if ( BT_thread_create ( &timer_tid,
                            NULL,
                            timer_start_routine,
                            NULL ) < 0 )
    {
        BT_TIMER_ERR(
        "FAILED to Create Timer Task.\n");
        return;
    }

    return;
}


void timer_bt_init ( void )
{
    UCHAR old_state;

    /* Lock Timer */
    timer_lock();

    BT_TIMER_TRC(
    "Bluetooth ON Initialization for Timer Library ...\n");

    /* Initialize Timer Q */
    timer_q_start = timer_q_end = NULL;

    old_state = timer_state;
    timer_state = TIMER_WAITING;

    if (old_state == TIMER_INIT)
    {
        timer_signal();
    }

    xTimerStart (base_timer_handle,0);
    timer_unlock();
    return;
}


void timer_bt_shutdown ( void )
{
    UINT16 index;
    TIMER_ENTITY *timer;
    UCHAR old_state;

    /* Lock Timer */
    timer_lock();

    xTimerStop (base_timer_handle,0);

    /* Initialize Timer Q */
    timer_q_start = timer_q_end = NULL;

    /* Initialize Timer Entities */
    for (index = 0; index < TIMER_MAX_ENTITIES; index ++)
    {
        timer = &timer_entity[index];
        if (TIMER_ENTITY_IN_USE == timer->in_use)
        {
#ifndef BT_NO_STATIC_DATA_SIZE
            if (timer->data_length > BT_STATIC_DATA_SIZE)
#endif /* BT_NO_STATIC_DATA_SIZE */
            {
                timer_free(timer->allocated_data);
            }

            timer_init_entity(timer);
        }
    }

    old_state = timer_state;
    timer_state = TIMER_SHUTDOWN;

    if (TIMER_WAITING == old_state)
    {
        timer_signal();
    }

    BT_TIMER_TRC(
    "Bluetooth OFF on Timer Library Module ...\n");

    timer_unlock();
    return;
}


API_RESULT BT_start_timer
           (
               BT_timer_handle *handle,
               UINT16 timeout,
               void (* callback) (void *, UINT16),
               void *data,
               UINT16 data_length
           )
{
    UCHAR *data_ptr = NULL;
    API_RESULT retval;
    TIMER_ENTITY * current_timer;

    if (NULL == handle)
    {
        BT_TIMER_ERR(
        "NULL Argument Unacceptable for Timer Handles.\n");

        return TIMER_HANDLE_IS_NULL;
    }

    BT_TIMER_TRC(
    "Preparing to Add New Timer Entity. Timeout = %d, Data Size = %d.\n",
    timeout, data_length);

    /* Timer Library expects to have a valid callback */
    if (NULL == callback)
    {
        BT_TIMER_ERR(
        "FAILED to Add New Timer Element. NULL Callback.\n");

        return TIMER_CALLBACK_IS_NULL;
    }

     /* Lock Timer */
    timer_lock();

    /* Intiialize Timer Entity */
    timer_allocate_entity (&current_timer);

    if (0 != data_length)
    {
#ifndef BT_NO_STATIC_DATA_SIZE
        if (data_length > BT_STATIC_DATA_SIZE)
#endif /* BT_NO_STATIC_DATA_SIZE */
        {
            data_ptr = (UCHAR *) timer_malloc (data_length);
            if (NULL == data_ptr)
            {
                BT_TIMER_ERR(
                "FAILED to allocate Memory for Timer Handler Argument.\n");

                return TIMER_MEMORY_ALLOCATION_FAILED;
            }
            current_timer->allocated_data = data_ptr;
        }
#ifndef BT_NO_STATIC_DATA_SIZE
        else
        {
            data_ptr = current_timer->static_data;
        }
#endif /* BT_NO_STATIC_DATA_SIZE */

        /* Copy the Data to be passed to the Timer Handler */
        BT_mem_copy(data_ptr, data, data_length);
    }

    /* Store Timer Data Length, Callback & Timeout */
    current_timer->callback = callback;
    current_timer->data_length = data_length;
    current_timer->timeout = timeout;
    current_timer->expire_at = current_tick + timeout;

    BT_TIMER_TRC(
    "New Timer Entity -> Scheduled to Expire at: %ld. Current Base Time: %ld.\n",
    (long int)current_timer->expire_at,
    (long int)(current_tick));

    /* Insert this Timer Entity into the List */
    retval = timer_add_entity(current_timer);

    if (API_SUCCESS != retval)
    {
        BT_TIMER_ERR(
        "FAILED to Add New Timer to Timer Queue. Error Code = 0x%04X\n",
        retval);

        timer_unlock();
        return retval;
    }

    /* Store the Handle */
    (*handle) = current_timer;

    BT_TIMER_TRC(
    "Succeesfully Added New Timer to Timer Queue. Handle = %p\n",
    (void *)*handle);

    /* Signal Timer Thread/Task to Run */
    if (TIMER_WAITING == timer_state)
    {
        BT_TIMER_TRC(
        "Signalling Timer Task/Thread to Run.\n");

        timer_state = TIMER_RUNNING;
        timer_signal();
    }

    timer_unlock();

    return API_SUCCESS;
}


API_RESULT BT_stop_timer
           (
               BT_timer_handle handle
           )
{
    TIMER_ENTITY *timer;
    API_RESULT retval;

    retval = API_SUCCESS;

    if (NULL == handle)
    {
        BT_TIMER_ERR(
        "NULL Argument Unacceptable for Timer Handles.\n");

        return TIMER_HANDLE_IS_NULL;
    }

    /* Lock Timer */
    timer_lock();

    timer = (TIMER_ENTITY *)handle;

    /* Check if the timer is already being freed - from the timer expairy handler */
    if (TIMER_ENTITY_IN_FREE == timer->in_use)
    {
        BT_TIMER_TRC (
        "[TIMER %p] Is already marked to be freed\n", timer);
    }
    else
    {
        retval = timer_del_entity(timer, TIMER_ENTITY_FREE_ALLOC_DATA);

        if (API_SUCCESS != retval)
        {
            BT_TIMER_ERR(
            "FAILED to find Timer Element. Handle = %p. Error Code = 0x%04X\n",
            (void *)handle, retval);
        }
        else
        {
            BT_TIMER_TRC(
            "Successfully Deleted Timer Element for Handle %p.\n",
            (void *)handle);
        }
    }

    /* Unlock Timer */
    timer_unlock();

    return retval;
}


API_RESULT BT_restart_timer
           (
               BT_timer_handle handle,
               UINT16 new_timeout
           )
{
    TIMER_ENTITY *timer;
    API_RESULT retval;

    if (NULL == handle)
    {
        BT_TIMER_ERR(
        "NULL Argument Unacceptable for Timer Handles.\n");

        return TIMER_HANDLE_IS_NULL;
    }

    /* Lock Timer */
    timer_lock();

    timer = (TIMER_ENTITY *)handle;

    retval = timer_search_entity(timer);

    if (API_SUCCESS != retval)
    {
        BT_TIMER_ERR(
        "FAILED to Find Timer ELement for Handle %p. Error Code = 0x%04X\n",
        (void *)handle, retval);
    }
    else
    {
        timer->timeout = new_timeout;
        timer->expire_at = current_tick + new_timeout;
    }

    timer_unlock();
    return retval;
}


API_RESULT BT_is_active_timer
           (
               BT_timer_handle handle
           )
{
    TIMER_ENTITY *timer;
    API_RESULT retval;

    if (NULL == handle)
    {
        BT_TIMER_ERR(
        "NULL Argument Unacceptable for Timer Handles.\n");

        return TIMER_HANDLE_IS_NULL;
    }

    timer_lock();

    timer = (TIMER_ENTITY *)handle;

    retval = timer_search_entity(timer);

    if (API_SUCCESS != retval)
    {
        BT_TIMER_ERR(
        "FAILED to Find the Timer Entity for Handle %p. Error Code = 0x%04X\n",
        (void *)handle, retval);
    }

    timer_unlock();

    return retval;
}


API_RESULT timer_search_entity ( TIMER_ENTITY *timer )
{
    TIMER_ENTITY *current_timer, *previous_timer;

    /* Is Queue Empty */
    if (NULL == timer_q_start)
    {
        return TIMER_QUEUE_EMPTY;
    }

    /* Handle the first Element */
    if (timer == timer_q_start)
    {
        return API_SUCCESS;
    }

    current_timer = timer_q_start->next;
    previous_timer = timer_q_start;

    while (NULL != current_timer)
    {
        if (timer == current_timer)
        {
            return API_SUCCESS;
        }

        previous_timer = current_timer;
        current_timer = current_timer->next;
    }

    return TIMER_ENTITY_SEARCH_FAILED;
}

API_RESULT timer_allocate_entity (BT_timer_handle *timer )
{   
    UINT16 index;
    API_RESULT retval;
    TIMER_ENTITY * new_timer;

    retval = API_SUCCESS;

    for (index = 0; index < TIMER_MAX_ENTITIES; index ++)
    {
        new_timer = &timer_entity[index];
        if (new_timer->in_use == 0x0)
        {
            new_timer->in_use = 0x1;
            break;
        }
        else
        {
            new_timer = NULL;
            retval = API_FAILURE;
        }
    }

    (*timer) = new_timer;
    return retval;
}


API_RESULT timer_add_entity ( TIMER_ENTITY *timer )
{
    TIMER_ENTITY *new_timer, *current_timer, *previous_timer;

    new_timer = timer;
    new_timer->next = NULL;

    /* If the Timer Q Empty */
    if (NULL == timer_q_start)
    {
        timer_q_start = timer_q_end = new_timer;
        return API_SUCCESS;
    }

    /* Timer Q has at least one element */
    if (timer_q_start->expire_at > new_timer->expire_at)
    {
        new_timer->next = timer_q_start;
        timer_q_start = new_timer;
        return API_SUCCESS;
    }

    previous_timer = timer_q_start;
    current_timer = timer_q_start->next;

    while (NULL != current_timer)
    {
        if (current_timer->expire_at > new_timer->expire_at)
        {
            previous_timer->next = new_timer;
            new_timer->next = current_timer;
            return API_SUCCESS;
        }

        previous_timer = current_timer;
        current_timer = current_timer->next;
    }

    timer_q_end->next = new_timer;
    timer_q_end = new_timer;

    return API_SUCCESS;
}


API_RESULT timer_del_entity
           (
               TIMER_ENTITY *timer,
               UCHAR free
           )
{
    TIMER_ENTITY *current_timer, *previous_timer;

    /* Either None or One Element */
    if (timer_q_start == timer_q_end)
    {
        if (NULL == timer_q_start)
        {
            /* Queue is Empty */
            return TIMER_QUEUE_EMPTY;
        }
        else
        {
            if (timer == timer_q_start)
            {
                /* Queue has One Element */
                timer_q_start = timer_q_end = NULL;
            }
            else
            {
                /* Match NOT found in the Only element in Timer Queue */
                return TIMER_ENTITY_SEARCH_FAILED;
            }
        }
    }
    else
    {
        /* Queue has more than One Element */

        if (timer == timer_q_start)
        {
            /* Match in the First Element */
            timer_q_start = timer_q_start->next;
        }
        else
        {
            previous_timer = timer_q_start;
            current_timer = timer_q_start->next;

            while (NULL != current_timer)
            {
                if (timer == current_timer)
                {
                    previous_timer->next = current_timer->next;

                    if (current_timer == timer_q_end)
                    {
                        timer_q_end = previous_timer;
                    }

                    break;
                }

                previous_timer = current_timer;
                current_timer  = current_timer->next;
            }

            if (NULL == current_timer)
            {
                return TIMER_ENTITY_SEARCH_FAILED;
            }
        }
    }

    /* Free Allocated Data */
    if ((0x01 == free)
#ifndef BT_NO_STATIC_DATA_SIZE
         && (timer->data_length > BT_STATIC_DATA_SIZE)
#endif /* BT_NO_STATIC_DATA_SIZE */
       )
    {
        timer_free (timer->allocated_data);
    }

    timer_init_entity(timer);
    return API_SUCCESS;
}


API_RESULT timer_init_entity (TIMER_ENTITY *timer)
{
    timer->in_use = TIMER_ENTITY_FREE;
    timer->timeout = 0;
    timer->expire_at = 0;
    timer->callback = NULL;
    timer->allocated_data = NULL;
    timer->data_length = 0;
    timer->next = NULL;

    return API_SUCCESS;
}


void timer_wait ( void )
{
    BT_thread_mutex_lock(&timer_cv_mutex);
    BT_thread_cond_wait(&timer_cv, &timer_cv_mutex);
    BT_thread_mutex_unlock(&timer_cv_mutex);
}

void timer_signal ( void )
{
    BT_thread_mutex_lock(&timer_cv_mutex);
    BT_thread_cond_signal(&timer_cv);
    BT_thread_mutex_unlock(&timer_cv_mutex);
}


void timer_sleep (UINT16 seconds)
{
    BT_sleep(seconds);
}


API_RESULT BT_list_timer ( void )
{
#ifdef BT_TIMERL_DEBUG

    UINT16 index, free;
    TIMER_ENTITY *timer;

    timer_lock();
    BT_TIMERL_TRC(
    "\n");
    BT_TIMERL_TRC(
    "========================================= \n");

    BT_TIMERL_TRC(
    "Start Q = %p\n", timer_q_start);

    timer = timer_q_start;
    while(timer != NULL)
    {
        BT_TIMERL_TRC(
        "    Handle = %p",
        timer->handle);

        BT_TIMERL_TRC(
        "        Expire At = %ld\n",
        timer->expire_at);

        timer = timer->next;
    }

    BT_TIMERL_TRC(
    "End   Q = %p\n", timer_q_end);

    free = 0;
    for (index = 0; index < TIMER_MAX_ENTITIES; index ++)
    {
        if (TIMER_ENTITY_FREE == timer_entity[index].in_use)
        {
            free ++;
        }
    }

    BT_TIMERL_TRC(
    "Max Q Entity = %d, Free = %d\n",
    TIMER_MAX_ENTITIES, free);

    BT_TIMERL_TRC(
    "========================================= \n");
    BT_TIMERL_TRC(
    "\n");

    timer_unlock();

#endif /* BT_TIMERL_DEBUG */

    return API_SUCCESS;
}

BT_THREAD_RETURN_TYPE timer_start_routine (BT_THREAD_ARGS args)
{
    BT_TIMERT_TRC(
    "Timer Task Start Routine: Starting Up ...\n");

    BT_LOOP_FOREVER()
    {
        /*
         *  Initial State for the Timer Task will be INIT, till
         *  timer_bt_init takes it to WAITING state.
         */
        if (TIMER_INIT == timer_state)
        {
            BT_TIMERT_TRC(
            "Current State is INIT. Waiting to be Initialized.\n");

            timer_wait();

            BT_TIMERT_TRC(
            "Timer Library Initialized. Timer Task preparing to Run.\n");
        }

        /* Timer Task got a chance to run */
        switch(timer_state)
        {
        case TIMER_INIT:
            BT_TIMERT_TRC(
            "EtherMind Timer Task - In INIT State ...\n");

            break;

        case TIMER_SHUTDOWN:
            timer_lock();
            timer_state = TIMER_INIT;
            timer_unlock();

            BT_TIMERT_TRC(
            "EtherMind Timer Task - Shutting Down ...\n");

            break;

        case TIMER_WAITING:
            BT_TIMERT_TRC(
            "EtherMind Timer Task - Waiting to service ...\n");

            timer_wait();

            break;

        case TIMER_RUNNING:     /* Fall Through */
        case TIMER_IMMEDIATE:
            BT_TIMERT_TRC(
            "EtherMind Timer Task - Ready to service ...\n");

            /* Now we have a request queued to be serviced */
            timer_service_queue();

            if (TIMER_RUNNING == timer_state)
            {
                timer_sleep(TIMER_TICK);
            }

            break;
        }
    }

    return BT_THREAD_RETURN_VAL_SUCCESS;
}


void timer_service_queue (void)
{
    UCHAR *data_ptr;
    TIMER_ENTITY *current_timer;
    UINT32 current_time;

    /* Lock Timer Library */
    timer_lock();

    /* Set State to RUNNING - explicitly */
    timer_state = TIMER_RUNNING;

    BT_TIMERT_TRC(
    "Timer Task: Servicing Timer Queue ...\n");

    /* Get Timer Element */
    current_timer = timer_q_start;

    if (NULL != current_timer)
    {
        /* Get Current Time */
        current_time = current_tick;

        BT_TIMERT_TRC(
        "Timer Task: Current Time is %ld.\n", (long int)current_time);

        if (current_time >= current_timer->expire_at)
        {
            data_ptr = NULL;

            /* Mark timer entity to be freed - once the callback returns */
            current_timer->in_use = TIMER_ENTITY_IN_FREE;
            if (0 != current_timer->data_length)
            {
#ifndef BT_NO_STATIC_DATA_SIZE
                data_ptr = (current_timer->data_length > BT_STATIC_DATA_SIZE)
                           ? current_timer->allocated_data
                           : current_timer->static_data;
#else
                data_ptr = current_timer->allocated_data;
#endif /* BT_NO_STATIC_DATA_SIZE */
            }

            BT_TIMERT_TRC(
            "Timer Task: Timeout for Handle %p. Enqueueing Request.\n",
            (void *)current_timer);

            /* Unlock Timer */
            timer_unlock();

            /*
             *  Timeout has occurred for this Timer element.
             *  Call the registered timeout handler
             */
            current_timer->callback
            (
                data_ptr,
                current_timer->data_length
            );

            /* Lock Timer */
            timer_lock();

            /* Initialize Timer Q Entity */
            timer_del_entity(current_timer, TIMER_ENTITY_FREE_ALLOC_DATA);

            /* We have to Handle the next one */
            timer_state = TIMER_IMMEDIATE;
        }
    }
    else
    {
        BT_TIMERT_TRC(
        "Timer Task: Timer Q Empty. Serviced All Entities. Returning Back.\n");

        /* We serviced all Q Elements - go back to WAITING State */
        timer_state = TIMER_WAITING;
    }

    timer_unlock();
    return;
}


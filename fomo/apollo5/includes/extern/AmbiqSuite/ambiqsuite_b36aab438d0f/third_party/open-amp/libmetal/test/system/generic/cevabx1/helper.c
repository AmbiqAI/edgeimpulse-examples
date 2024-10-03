/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * All rights reserved.
 *
 * Copyright (c) 2016 Xilinx, Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
/* Main hw machinery initialization entry point, called from main()*/
/* return 0 on success */

int metal_irq_init(void);

int init_system(void)
{
    metal_irq_init();
    return 0;
}

/*
 * Copyright (c) 2011-2012, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <mach/rpm-regulator.h>
#include <mach/msm_bus_board.h>
#include <mach/msm_bus.h>

#include "acpuclock.h"
#include "acpuclock-krait.h"

/* Corner type vreg VDD values */
#define LVL_NONE	RPM_VREG_CORNER_NONE
#define LVL_LOW		RPM_VREG_CORNER_LOW
#define LVL_NOM		RPM_VREG_CORNER_NOMINAL
#define LVL_HIGH	RPM_VREG_CORNER_HIGH

static struct hfpll_data hfpll_data = {
	.mode_offset = 0x00,
	.l_offset = 0x08,
	.m_offset = 0x0C,
	.n_offset = 0x10,
	.config_offset = 0x04,
	.config_val = 0x7845C665,
	.has_droop_ctl = true,
	.droop_offset = 0x14,
	.droop_val = 0x0108C000,
	.low_vdd_l_max = 40,
	.vdd[HFPLL_VDD_NONE] = LVL_NONE,
	.vdd[HFPLL_VDD_LOW]  = LVL_LOW,
	.vdd[HFPLL_VDD_NOM]  = LVL_NOM,
};

static struct scalable scalable[] = {
	[CPU0] = {
		.hfpll_phys_base = 0x00903200,
		.hfpll_data = &hfpll_data,
		.aux_clk_sel_phys = 0x02088014,
		.aux_clk_sel = 3,
		.l2cpmr_iaddr = 0x4501,
		.vreg[VREG_CORE] = { "krait0", 1300000, 1740000 },
		.vreg[VREG_MEM]  = { "krait0_mem", 1150000 },
		.vreg[VREG_DIG]  = { "krait0_dig", 1150000 },
		.vreg[VREG_HFPLL_A] = { "krait0_hfpll", 1800000 },
	},
	[CPU1] = {
		.hfpll_phys_base = 0x00903300,
		.hfpll_data = &hfpll_data,
		.aux_clk_sel_phys = 0x02098014,
		.aux_clk_sel = 3,
		.l2cpmr_iaddr = 0x5501,
		.vreg[VREG_CORE] = { "krait1", 1300000, 1740000 },
		.vreg[VREG_MEM]  = { "krait1_mem", 1150000 },
		.vreg[VREG_DIG]  = { "krait1_dig", 1150000 },
		.vreg[VREG_HFPLL_A] = { "krait1_hfpll", 1800000 },
	},
	[L2] = {
		.hfpll_phys_base = 0x00903400,
		.hfpll_data = &hfpll_data,
		.aux_clk_sel_phys = 0x02011028,
		.aux_clk_sel = 3,
		.l2cpmr_iaddr = 0x0500,
		.vreg[VREG_HFPLL_A] = { "l2_hfpll", 1800000 },
	},
};

static struct msm_bus_paths bw_level_tbl[] = {
	[0] =  BW_MBPS(640), /* At least  80 MHz on bus. */
	[1] = BW_MBPS(1064), /* At least 133 MHz on bus. */
	[2] = BW_MBPS(1600), /* At least 200 MHz on bus. */
	[3] = BW_MBPS(2128), /* At least 266 MHz on bus. */
	[4] = BW_MBPS(3200), /* At least 400 MHz on bus. */
	[5] = BW_MBPS(3600), /* At least 450 MHz on bus. */
	[6] = BW_MBPS(3936), /* At least 492 MHz on bus. */
	[7] = BW_MBPS(4264), /* At least 533 MHz on bus. */
};

static struct msm_bus_scale_pdata bus_scale_data = {
	.usecase = bw_level_tbl,
	.num_usecases = ARRAY_SIZE(bw_level_tbl),
	.active_only = 1,
	.name = "acpuclk-8930",
};

/* TODO: Update vdd_dig, vdd_mem and bw when data is available. */
#define L2(x) (&l2_freq_tbl[(x)])
static struct l2_level l2_freq_tbl[] = {
	[0]  = { {STBY_KHZ, QSB,   0, 0, 0x00 },  LVL_NOM, 1050000, 0 },
	[1]  = { {  384000, PLL_8, 0, 2, 0x00 },  LVL_NOM, 1050000, 1 },
	[2]  = { {  432000, HFPLL, 2, 0, 0x20 },  LVL_NOM, 1050000, 2 },
	[3]  = { {  486000, HFPLL, 2, 0, 0x24 },  LVL_NOM, 1050000, 2 },
	[4]  = { {  540000, HFPLL, 2, 0, 0x28 },  LVL_NOM, 1050000, 2 },
	[5]  = { {  594000, HFPLL, 1, 0, 0x16 },  LVL_NOM, 1050000, 2 },
	[6]  = { {  648000, HFPLL, 1, 0, 0x18 },  LVL_NOM, 1050000, 4 },
	[7]  = { {  702000, HFPLL, 1, 0, 0x1A },  LVL_NOM, 1050000, 4 },
	[8]  = { {  756000, HFPLL, 1, 0, 0x1C }, LVL_HIGH, 1150000, 4 },
	[9]  = { {  810000, HFPLL, 1, 0, 0x1E }, LVL_HIGH, 1150000, 4 },
	[10] = { {  864000, HFPLL, 1, 0, 0x20 }, LVL_HIGH, 1150000, 4 },
	[11] = { {  918000, HFPLL, 1, 0, 0x22 }, LVL_HIGH, 1150000, 7 },
	[12] = { {  972000, HFPLL, 1, 0, 0x24 }, LVL_HIGH, 1150000, 7 },
	[13] = { { 1026000, HFPLL, 1, 0, 0x26 }, LVL_HIGH, 1150000, 7 },
	[14] = { { 1080000, HFPLL, 1, 0, 0x28 }, LVL_HIGH, 1150000, 7 },
	[15] = { { 1134000, HFPLL, 1, 0, 0x2A }, LVL_HIGH, 1150000, 7 },
	[16] = { { 1188000, HFPLL, 1, 0, 0x2C }, LVL_HIGH, 1150000, 7 },
};

static struct acpu_level acpu_freq_tbl_slow[] = {
	{ 0, { STBY_KHZ, QSB,   0, 0, 0x00 }, L2(0),   950000 },
	{ 1, {   384000, PLL_8, 0, 2, 0x00 }, L2(1),   950000 },
	{ 1, {   432000, HFPLL, 2, 0, 0x20 }, L2(6),   975000 },
	{ 1, {   486000, HFPLL, 2, 0, 0x24 }, L2(6),   975000 },
	{ 1, {   540000, HFPLL, 2, 0, 0x28 }, L2(6),  1000000 },
	{ 1, {   594000, HFPLL, 1, 0, 0x16 }, L2(6),  1000000 },
	{ 1, {   648000, HFPLL, 1, 0, 0x18 }, L2(6),  1025000 },
	{ 1, {   702000, HFPLL, 1, 0, 0x1A }, L2(6),  1025000 },
	{ 1, {   756000, HFPLL, 1, 0, 0x1C }, L2(11), 1075000 },
	{ 1, {   810000, HFPLL, 1, 0, 0x1E }, L2(11), 1075000 },
	{ 1, {   864000, HFPLL, 1, 0, 0x20 }, L2(11), 1100000 },
	{ 1, {   918000, HFPLL, 1, 0, 0x22 }, L2(11), 1100000 },
	{ 1, {   972000, HFPLL, 1, 0, 0x24 }, L2(11), 1125000 },
	{ 1, {  1026000, HFPLL, 1, 0, 0x26 }, L2(11), 1125000 },
	{ 1, {  1080000, HFPLL, 1, 0, 0x28 }, L2(16), 1175000 },
	{ 1, {  1134000, HFPLL, 1, 0, 0x2A }, L2(16), 1175000 },
	{ 1, {  1188000, HFPLL, 1, 0, 0x2C }, L2(16), 1200000 },
	{ 0, { 0 } }
};

static struct acpu_level acpu_freq_tbl_nom[] = {
	{ 0, { STBY_KHZ, QSB,   0, 0, 0x00 }, L2(0),   925000 },
	{ 1, {   384000, PLL_8, 0, 2, 0x00 }, L2(1),   925000 },
	{ 1, {   432000, HFPLL, 2, 0, 0x20 }, L2(6),   950000 },
	{ 1, {   486000, HFPLL, 2, 0, 0x24 }, L2(6),   950000 },
	{ 1, {   540000, HFPLL, 2, 0, 0x28 }, L2(6),   975000 },
	{ 1, {   594000, HFPLL, 1, 0, 0x16 }, L2(6),   975000 },
	{ 1, {   648000, HFPLL, 1, 0, 0x18 }, L2(6),  1000000 },
	{ 1, {   702000, HFPLL, 1, 0, 0x1A }, L2(6),  1000000 },
	{ 1, {   756000, HFPLL, 1, 0, 0x1C }, L2(11), 1050000 },
	{ 1, {   810000, HFPLL, 1, 0, 0x1E }, L2(11), 1050000 },
	{ 1, {   864000, HFPLL, 1, 0, 0x20 }, L2(11), 1075000 },
	{ 1, {   918000, HFPLL, 1, 0, 0x22 }, L2(11), 1075000 },
	{ 1, {   972000, HFPLL, 1, 0, 0x24 }, L2(11), 1100000 },
	{ 1, {  1026000, HFPLL, 1, 0, 0x26 }, L2(11), 1100000 },
	{ 1, {  1080000, HFPLL, 1, 0, 0x28 }, L2(16), 1150000 },
	{ 1, {  1134000, HFPLL, 1, 0, 0x2A }, L2(16), 1150000 },
	{ 1, {  1188000, HFPLL, 1, 0, 0x2C }, L2(16), 1175000 },
	{ 0, { 0 } }
};

static struct acpu_level acpu_freq_tbl_fast[] = {
	{ 0, { STBY_KHZ, QSB,   0, 0, 0x00 }, L2(0),   900000 },
	{ 1, {   384000, PLL_8, 0, 2, 0x00 }, L2(1),   900000 },
	{ 1, {   432000, HFPLL, 2, 0, 0x20 }, L2(6),   900000 },
	{ 1, {   486000, HFPLL, 2, 0, 0x24 }, L2(6),   900000 },
	{ 1, {   540000, HFPLL, 2, 0, 0x28 }, L2(6),   925000 },
	{ 1, {   594000, HFPLL, 1, 0, 0x16 }, L2(6),   925000 },
	{ 1, {   648000, HFPLL, 1, 0, 0x18 }, L2(6),   950000 },
	{ 1, {   702000, HFPLL, 1, 0, 0x1A }, L2(6),   950000 },
	{ 1, {   756000, HFPLL, 1, 0, 0x1C }, L2(11), 1000000 },
	{ 1, {   810000, HFPLL, 1, 0, 0x1E }, L2(11), 1000000 },
	{ 1, {   864000, HFPLL, 1, 0, 0x20 }, L2(11), 1025000 },
	{ 1, {   918000, HFPLL, 1, 0, 0x22 }, L2(11), 1025000 },
	{ 1, {   972000, HFPLL, 1, 0, 0x24 }, L2(11), 1050000 },
	{ 1, {  1026000, HFPLL, 1, 0, 0x26 }, L2(11), 1050000 },
	{ 1, {  1080000, HFPLL, 1, 0, 0x28 }, L2(16), 1100000 },
	{ 1, {  1134000, HFPLL, 1, 0, 0x2A }, L2(16), 1100000 },
	{ 1, {  1188000, HFPLL, 1, 0, 0x2C }, L2(16), 1125000 },
	{ 0, { 0 } }
};

static struct acpuclk_krait_params acpuclk_8930_params = {
	.scalable = scalable,
	.pvs_acpu_freq_tbl[PVS_SLOW] = acpu_freq_tbl_slow,
	.pvs_acpu_freq_tbl[PVS_NOMINAL] = acpu_freq_tbl_nom,
	.pvs_acpu_freq_tbl[PVS_FAST] = acpu_freq_tbl_fast,
	.l2_freq_tbl = l2_freq_tbl,
	.l2_freq_tbl_size = ARRAY_SIZE(l2_freq_tbl),
	.bus_scale_data = &bus_scale_data,
	.qfprom_phys_base = 0x00700000,
};

static int __init acpuclk_8930_probe(struct platform_device *pdev)
{
	return acpuclk_krait_init(&pdev->dev, &acpuclk_8930_params);
}

static struct platform_driver acpuclk_8930_driver = {
	.driver = {
		.name = "acpuclk-8930",
		.owner = THIS_MODULE,
	},
};

static int __init acpuclk_8930_init(void)
{
	return platform_driver_probe(&acpuclk_8930_driver,
				     acpuclk_8930_probe);
}
device_initcall(acpuclk_8930_init);
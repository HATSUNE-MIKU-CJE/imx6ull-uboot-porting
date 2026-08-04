#include <phy.h>
#include <miiphy.h>

/* PHY ID */
#define PHY_SR8201F_ID    0x1CC816
#define PHY_SR8201F_MASK  0xFFFFFFF0

/* 页选择寄存器（寄存器31）*/
#define SR8201F_PAGE_REG  31
#define SR8201F_RMII_PAGE 0x07

/* RMII Mode Setting Register（寄存器16，Page 7）*/
#define SR8201F_RMSR      16

/* 页切换函数 把page写入寄存器31 */
static void sr8201f_page_select(struct phy_device *phydev, int page)
{
    phy_write(phydev, MDIO_DEVAD_NONE, SR8201F_PAGE_REG, page);
}

static int sr8201f_config(struct phy_device *phydev)
{
    u16 reg;
    sr8201f_page_select(phydev, SR8201F_RMII_PAGE);
    reg = phy_read(phydev, MDIO_DEVAD_NONE, SR8201F_RMSR);
    printf("SR8201F: RMSR = 0x%04x\n", reg);
    sr8201f_page_select(phydev, 0);
    return 0;
}

static int sr8201f_parse_status(struct phy_device *phydev)
{
	int mii_reg;

	mii_reg = phy_read(phydev, MDIO_DEVAD_NONE, MII_BMSR);

	if (mii_reg & (BMSR_100FULL | BMSR_100HALF))
		phydev->speed = SPEED_100;
	else
		phydev->speed = SPEED_10;

	if (mii_reg & (BMSR_10FULL | BMSR_100FULL))
		phydev->duplex = DUPLEX_FULL;
	else
		phydev->duplex = DUPLEX_HALF;

	return 0;
}

static int sr8201f_startup(struct phy_device *phydev)
{
	genphy_update_link(phydev);
	sr8201f_parse_status(phydev);
	return 0;
}

static struct phy_driver sr8201f_driver = {
    .name = "SR8201F",
    .uid  = PHY_SR8201F_ID,
    .mask = PHY_SR8201F_MASK,
    .features = PHY_BASIC_FEATURES,
    .config  = &sr8201f_config,
    .startup = &sr8201f_startup,
    .shutdown = &genphy_shutdown,
};

int phy_sr8201f_init(void)
{
    phy_register(&sr8201f_driver);
    return 0;
}
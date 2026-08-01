# U-Boot 移植 —— 正点原子 i.MX6ULL Alpha (EMMC)

将 NXP 官方 U-Boot（`imx_4.1.15_2.1.0_ga`）移植到正点原子 I.MX6ULL Alpha V2.8 开发板（EMMC 版），
新增 `mx6ull_alpha` 开发板支持。

## 当前状态

| 外设 | 状态 |
|------|:--:|
| 开发板识别 | ✅ `Board: MX6ULL 14x14 ALPHA` |
| DDR | ✅ 512 MiB |
| SD/MMC | ✅ FSL_SDHC: 0, 1 |
| 串口 | ✅ 正常 |
| LCD (TFT7016) | ❌ 暂不支持 |
| 网络 (PHY SR8201F) | ❌ 待适配 |

## 环境

| 项 | 版本 |
|------|------|
| 宿主机 | Windows 11 + WSL2 |
| Linux | Ubuntu 24.04 |
| 交叉编译器 | arm-linux-gnueabihf-gcc (Ubuntu 13.3.0) |
| 目标板 | 正点原子 IMX6ULL Alpha V2.8 (EMMC) |
| PHY 芯片 | SR8201F |

## 移植内容

以官方 `mx6ull_14x14_evk` 为参考，新增 `mx6ull_alpha` 支持。U-Boot 识别一块新板子需要四层协作：

| 层次 | 文件 | 作用 |
|------|------|------|
| 报名表 | `configs/mx6ull_alpha_emmc_defconfig` | 默认配置（门牌号） |
| 登记表 | `arch/arm/cpu/armv7/mx6/Kconfig` | 注册 `TARGET_MX6ULL_ALPHA` 并 source 板级目录 |
| 装备 | `board/freescale/mx6ull_alpha/` | 板级硬件初始化源码 |
| 配置头文件 | `include/configs/mx6ull_alpha.h` | 内存/时钟/外设等宏配置 |

源码改动：

- `arch/arm/Makefile`：`armv5` → `armv5te` + 加 `-mfpu=neon`（GCC13 兼容）
- `Makefile` / `.config`：`CONFIG_PHYLIB=y`

## 编译

```bash
cd <本仓库目录>
make mx6ull_alpha_emmc_defconfig
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j$(nproc)
```

## 烧写（WSL2 + usbipd）

读卡器透传给 WSL2（Windows 管理员 PowerShell）：

```powershell
usbipd list                          # 找到读卡器 BUSID
usbipd bind --busid <BUSID>          # 首次绑定
usbipd attach --wsl --busid <BUSID>  # 每次重启 WSL 后执行
```

Linux 侧烧写（`imxdownload` 工具需与编译产物同目录）：

```bash
lsblk                                # 确认读卡器设备名，如 /dev/sde
chmod +x imxdownload
./imxdownload u-boot.bin /dev/sdX    # 用 u-boot.bin，不是 .imx；设备是整卡，不是分区
```

> `imxdownload` 自动为 `u-boot.bin` 加上 IVT/DCD 头部生成 `load.imx` 并写入 SD 卡。前两个扇区是分区表，不会被覆盖。

## 踩坑记录

| 问题 | 修复 |
|------|------|
| GCC13 报 `unrecognized -march target: armv5` | `arch/arm/Makefile` 改为 `armv5te` + `-mfpu=neon` |
| `CONFIG_PHYLIB` 未选导致网络初始化失败 | `.config` 手动加 `CONFIG_PHYLIB=y` |
| 复制 board 目录后残留 `.o/.su/.cfgtmp` | 移植前必须清理，否则编译系统可能跳过 |
| ping 时 data abort 崩溃 | `start.S` SCTLR bit[1] `orr` → `bic` |
| RX error frame（FEC_RBD_NO） | PHY 复位延迟 20→100ms、150→250ms |
| PHY SR8201F 驱动不匹配 | 待驱动篇学习完成后解决 |

## 已知问题

- **LCD（TFT7016）**：启动信息报 `unsupported panel TFT7016`，待适配
- **网络（SR8201F）**：`Board Net Initialization Failed`，PHY 驱动与 SR8201F 不匹配，遗留

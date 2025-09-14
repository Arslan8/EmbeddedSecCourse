
# STM32F429 Initialization — Self-Contained Intro Notes
## 1. Overview 
On reset the STM32F429 runs from a default, conservative clock and minimal power configuration. Before running your application at full speed or using peripherals, you must: (1) enable caches and Flash settings for correct and performant instruction fetches, (2) enable peripheral clocks for modules you will configure (PWR, SYSCFG, GPIO, etc.), (3) pick and enable a clock source (HSI, HSE, PLL), (4) set voltage/regulator scaling to support your target speed, (5) configure Flash latency, prescalers, and then switch SYSCLK to the final source. Doing these steps in the right order avoids crashes and unpredictable behavior.

---

## 2. Glossary (quick definitions)

- **ACR** — Flash *Access Control Register*. Controls Flash prefetch, instruction cache, data cache, and latency (wait-states) for Flash memory.
- **RCC** — *Reset and Clock Control*. The unit that controls oscillators (HSI/HSE/PLL), prescalers, and peripheral clock gating (APB/AHB).
- **AHB** — *Advanced High-performance Bus*. The high-speed bus used by core, SRAM, DMA, etc.
- **APB1 / APB2** — *Advanced Peripheral Bus 1/2*. Buses for peripherals. APB1 is lower-speed; APB2 is higher-speed. Peripherals live on these buses and must have their clock enabled to access their registers.
- **PWR** — *Power Control peripheral*. Lets firmware change internal regulator settings (VOS), manage low-power modes and overdrive features.
- **SYSCFG** — *System Configuration Controller*. System-level features such as EXTI line mapping, memory remap, and some I/O compensation controls.
- **HSI / HSE** — Internal (HSI) or external (HSE) high-speed oscillators. HSI is typically 16 MHz RC; HSE is your external crystal/oscillator (commonly 8 MHz).
- **PLL** — Phase-Locked Loop: used to multiply a base oscillator frequency to a higher SYSCLK frequency (e.g., 168 MHz).
- **VOS** — Voltage scaling selection for the internal regulator; different VOS levels permit different maximum CPU frequencies.
- **Flash latency (wait states)** — Number of clock cycles the CPU must wait when reading Flash; higher SYSCLK needs more wait states to allow Flash to deliver data.

---

## 3. Why each building block is needed (reasoning / intuition)

### 3.1 Flash and caches (ACR)
- **Why it exists:** The Flash memory that stores program code is much slower than the CPU core. The MCU provides instruction and data caches and prefetch to bridge this gap. The ACR register controls these features and flash wait states.
- **Why we enable caches early:** To reduce stalls and make startup code run faster. However, when you reflash the device, caches may need invalidation to avoid stale data usage.

### 3.2 Peripheral clock gating (RCC APB/APB2 enables)
- **Why it exists:** To save power and keep bus traffic under control. Peripherals must be explicitly enabled in RCC before you can safely read/write their registers. The enable bit is per-peripheral and located in APB1ENR, APB2ENR, AHBENR, etc.
- **Why we read-back after write:** Many HAL macros write an enable bit then immediately READ the same register into a `tmpreg` variable. The read forces the write to complete on the bus fabric and avoids race conditions where the peripheral is not yet clocked when you access it.

### 3.3 PWR and voltage scaling (VOS)
- **Why it exists:** Digital logic requires sufficient supply voltage to switch reliably at higher frequencies. If you drive the clock faster than the voltage allows, the chip may misbehave or crash. The PWR peripheral lets firmware choose the VOS level that ensures stable operation at the target SYSCLK.
- **Best practice:** Set VOS before increasing the clock frequency.

### 3.4 Oscillators and PLL
- **Why it exists:** The internal HSI is cheap and available at reset. For precise timing or higher frequencies, you enable HSE (crystal) and configure the PLL to multiply the base clock up to the desired SYSCLK.
- **Important:** Always wait for oscillator/PLL ready flags (`HSERDY`, `HSIRDY`, `PLLRDY`) before switching to them.

### 3.5 Flash latency and prescalers
- **Why it exists:** Flash memory has access timing limits. The CPU clock must not outpace the Flash's ability to deliver instructions. Flash latency (wait-states) protects against this. Prescalers keep APB1/APB2/AXI/AHB buses within their rated maximum frequencies.

---

## 4. Common failures and what they look like (diagnostic reasoning)
- **Too-low flash latency for new SYSCLK:** Random crashes, hard faults, wrong instruction execution. Symptom often occurs immediately after clock change.
- **Changed clocks but did not set VOS:** System may reset or hang when speed increases.
- **Wrote to PWR register before enabling its clock:** Writes are ignored; subsequent VOS change has no effect.
- **Accessed peripheral immediately after enabling its RCC bit (no readback):** Rare race conditions where the peripheral behaves as if it is disabled. The readback prevents that.

---

## 5. Minimal safe "HSI-only" initialization (example)

This is a simple, safe configuration that runs from the internal HSI at 16 MHz. Useful for bootloaders, quick tests, or when you don't need high performance.

```c
void SystemClock_Config_HSI(void)
{
    // 0) Optional: enable CPU caches (CMSIS helpers)
    SCB_EnableICache();
    SCB_EnableDCache();

    // 1) Enable PWR clock so we can set voltage scaling (safe habit)
    __HAL_RCC_PWR_CLK_ENABLE();

    // 2) Set voltage scaling for normal full-speed operation if you plan to increase later
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    // 3) Configure and enable HSI (16 MHz), no PLL
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) { Error_Handler(); }

    // 4) Select HSI as SYSCLK and set prescalers to 1
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    // FLASH_LATENCY_0 is appropriate for 16 MHz
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK) { Error_Handler(); }
}
```

**Why use HSI-only example:** Simple, predictable, good for early boot stages or when implementing PLL configuration later in a controlled manner.

---

## 6. Example: Full performance init — HSE + PLL -> 168 MHz (typical example)
> **Warning:** This example assumes a common 8 MHz external crystal (HSE = 8 MHz). If your board uses a different HSE, adjust PLL parameters accordingly. Always consult the device reference manual for correct PLL parameter ranges and flash latency mapping.

```c
void SystemClock_Config_168MHz(void)
{
    // Enable caches
    SCB_EnableICache();
    SCB_EnableDCache();

    // 1) Ensure PWR clock is enabled to allow VOS changes
    __HAL_RCC_PWR_CLK_ENABLE();

    // 2) Set regulator voltage scale to Scale 1 for high frequency operation
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    // 3) Turn on HSE and configure PLL (example for 8 MHz HSE -> 168 MHz SYSCLK)
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

    // PLL parameters (common example for 8 MHz input)
    RCC_OscInitStruct.PLL.PLLM = 8;    // VCO input = 8 MHz / 8 = 1 MHz
    RCC_OscInitStruct.PLL.PLLN = 336;  // VCO output = 1 MHz * 336 = 336 MHz
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2; // SYSCLK = 336 / 2 = 168 MHz
    RCC_OscInitStruct.PLL.PLLQ = 7;    // For USB OTG FS clock (if needed)

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) { Error_Handler(); }

    // 4) Configure prescalers and flash latency BEFORE switching to PLL
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;    // HCLK = SYSCLK / 1
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;     // APB1 max 42 MHz -> divide by 4
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;     // APB2 max 84 MHz -> divide by 2

    // Choose FLASH_LATENCY according to final SYSCLK (e.g., FLASH_LATENCY_5 for 168 MHz)
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) { Error_Handler(); }
}
```

**Reasoning notes for this example:**  
- We set VOS to Scale1 **before** enabling the PLL to ensure the regulator can support 168 MHz.  
- We set APB prescalers so APB1 and APB2 remain within their rated limits (APB1 ≤ 42 MHz, APB2 ≤ 84 MHz for many F4 parts). The AHB can be at 168 MHz.  
- `HAL_RCC_OscConfig` will wait for HSE/PLL ready internally; HAL handles readiness polling for you.

---

## 7. Full "self-contained" checklist (one-page)
Use this when teaching or when writing startup code:

1. Optional: Invalidate caches if reprogramming flash often. Then enable I/D caches:
   - `SCB_DisableDCache(); SCB_DisableICache();` then re-enable after flash programming.
   - Or use `SCB_EnableICache(); SCB_EnableDCache();` at startup.
2. `__HAL_RCC_PWR_CLK_ENABLE()` — enable PWR peripheral clock.
3. `__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1)` — set VOS (choose based on target SYSCLK).
4. Configure oscillator (HSI or HSE + PLL) using `HAL_RCC_OscConfig` and check ready flags. Example: HSE on + PLL on.
5. Choose AHB/APB prescalers to keep peripheral buses under max frequency.
6. Set Flash latency appropriate for final SYSCLK. (Use HAL macro `FLASH_LATENCY_x` or check RM table.)
7. Switch SYSCLK to final clock source using `HAL_RCC_ClockConfig` (HAL does latencies and checks for you).
8. Enable specific peripheral clocks (GPIO, SYSCFG, USART, etc.) when you plan to access them:
   - `__HAL_RCC_GPIOA_CLK_ENABLE(); __HAL_RCC_SYSCFG_CLK_ENABLE();` etc.
9. Update `SystemCoreClock` (CMSIS/HAL usually does this) if needed.

---

## 8. Debugging tips and useful checks (self-contained)
- **Check oscillator ready bits:** `if (RCC->CR & RCC_CR_HSERDY) { /* HSE ready */ }`
- **Check PLL ready:** `if (RCC->CR & RCC_CR_PLLRDY) { /* PLL locked */ }`
- **Check SYSCLK source:** read `RCC->CFGR & RCC_CFGR_SWS` to find current source.
- **Check Flash latency register:** `FLASH->ACR & FLASH_ACR_LATENCY` to confirm wait states.
- **If system faults occur after clock change:** halt in debugger and inspect `CFSR` and `HFSR` fault registers; if faults happen at same instruction repeatedly, suspect Flash latency or cache corruption.
- **Use LED/GPIO toggle to verify clock frequency:** Toggle a pin from a timer to observe expected frequency on an oscilloscope.

---

## 9. References & further reading (self-contained pointers)
- **Device Reference Manual** (search for “STM32F429 reference manual RM0090”) — contains the authoritative tables mapping frequency to Flash latency and VOS requirements.  
- **Datasheet** for maximum guaranteed frequencies and voltage/current characteristics.  
- **CMSIS & HAL code** — look at `system_stm32f4xx.c`, `stm32f4xx_hal_rcc.c` to see real HAL sequences and helper macros.

---


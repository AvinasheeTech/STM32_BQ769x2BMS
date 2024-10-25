/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           main.c
  * @brief          main program to implement 6S BMS with STM32F4
  * @author         Avinashee Tech
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "BMS_SPI.h"
#include "BQ76942.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define RCC_PLLCFGR_PLLM_ 4
#define RCC_PLLCFGR_PLLN_ 168
#define RCC_PLLCFGR_PLLQ_ 7
#define RCC_PLLCFGR_PLLR_ 2

#define SPI_CRC_POLY_     7
#define TIM7_PSC_         8400
#define TIM7_ARR_         9

#define AHB_Prescaler_    0
#define APB1_Prescaler_   5
#define APB2_Prescaler_   4
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
volatile uint64_t counter_millis = 0;
const uint8_t uwTickFreq = 1;
uint32_t uwTickPrio   = (1UL << __NVIC_PRIO_BITS); /* Invalid PRIO */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void TIM7_IRQ_handler(void);
uint32_t RCC_GetSysClockFreq(void);
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  /*****************Flash control section*****************/
  SET_BIT(FLASH->ACR,FLASH_ACR_ICEN | FLASH_ACR_PRFTEN | FLASH_ACR_DCEN);

  /*****************Systick section************************/
  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
  // Configure the SysTick to have interrupt in 1ms time basis
  SysTick_Config((SystemCoreClock / (1000U / uwTickFreq)) > 0U);
  uint32_t prioritygroup = 0x00U;
  prioritygroup = NVIC_GetPriorityGrouping();
  NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(prioritygroup, 0U, 0U));
  uwTickPrio = 0U;

  //init low level hardware
  SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN);
  SET_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN);
  CLEAR_BIT(PWR->CR,PWR_CR_VOS);
  SET_BIT(PWR->CR,PWR_CR_VOS);

  /**********************HSE section***********************/
  //clear HSE ready interrupt
  SET_BIT(RCC->CIR,RCC_CIR_HSERDYC);

  //enable HSE ready interrupt
  SET_BIT(RCC->CIR,RCC_CIR_HSERDYIE);

  //HSE on
  SET_BIT(RCC->CR,RCC_CR_HSEBYP);
  SET_BIT(RCC->CR,RCC_CR_HSEON);

  //wait until HSE is ready

  while((READ_BIT(RCC->CR,RCC_CR_HSERDY)==0U) | (READ_BIT(RCC->CIR,RCC_CIR_HSERDYF)==0U) ){
	  //wait until HSE ready interrupt is set
  }

  //clear HSE ready interrupt
  SET_BIT(RCC->CIR,RCC_CIR_HSERDYC);

  /**********************PLL section**********************/
  //clear PLL ready interrupt
  SET_BIT(RCC->CIR,RCC_CIR_PLLRDYC);

  //enable PLL ready interrupt
  SET_BIT(RCC->CIR,RCC_CIR_PLLRDYIE);

  //PLL disable
  CLEAR_BIT(RCC->CR,RCC_CR_PLLON);

  CLEAR_BIT(RCC->PLLCFGR,RCC_PLLCFGR_PLLM);
  CLEAR_BIT(RCC->PLLCFGR,RCC_PLLCFGR_PLLN);
  CLEAR_BIT(RCC->PLLCFGR,RCC_PLLCFGR_PLLQ);
  CLEAR_BIT(RCC->PLLCFGR,RCC_PLLCFGR_PLLP);

  //PLL config
  SET_BIT(RCC->PLLCFGR,(RCC_PLLCFGR_PLLM_<<0U) | (RCC_PLLCFGR_PLLN_<<6U) |
		  (RCC_PLLCFGR_PLLQ_<<24U) | RCC_PLLCFGR_PLLSRC_HSE);


  //PLL enable
  SET_BIT(RCC->CR,RCC_CR_PLLON);

  //wait until PLL is ready
  while((READ_BIT(RCC->CR,RCC_CR_PLLRDY)==0U) | (READ_BIT(RCC->CIR,RCC_CIR_PLLRDYF)==0U) ){
 	  //nop
  }

  //clear PLL ready interrupt
  SET_BIT(RCC->CIR,RCC_CIR_PLLRDYC);

  /***************************Flash Latency************************/
  if(FLASH_ACR_LATENCY_5WS>(READ_REG(FLASH->ACR) & (0xFU << FLASH_ACR_LATENCY_Pos))){
	  SET_BIT(FLASH->ACR,FLASH_ACR_LATENCY_5WS);
	  while((READ_REG(FLASH->ACR) & (0xFU << 0U))!=FLASH_ACR_LATENCY_5WS){
		  //wait until changes are reflected
	  }

  }

  /**********************APB1 CLK**************************/
  CLEAR_BIT(RCC->CFGR,RCC_CFGR_PPRE1);
  SET_BIT(RCC->CFGR,RCC_CFGR_PPRE1_DIV16);

  /**********************APB2 CLK**************************/
  CLEAR_BIT(RCC->CFGR,RCC_CFGR_PPRE2);
  SET_BIT(RCC->CFGR,RCC_CFGR_PPRE2_DIV16);

  /***********************AHB CLK**************************/
  CLEAR_BIT(RCC->CFGR,RCC_CFGR_HPRE);

  /*********************System Clock section***************/
  //system clock setting
  CLEAR_BIT(RCC->CFGR,RCC_CFGR_SW);
  SET_BIT(RCC->CFGR,RCC_CFGR_SW_PLL);

  while((READ_REG(RCC->CFGR) & (0x3U<<2U))!=RCC_CFGR_SWS_PLL){
	  //check if system clock is switched correctly
  }

  /**********************APB1 CLK**************************/
  CLEAR_BIT(RCC->CFGR,RCC_CFGR_PPRE1);
  SET_BIT(RCC->CFGR,(APB1_Prescaler_<<RCC_CFGR_PPRE1_Pos));

  /**********************APB2 CLK**************************/
  CLEAR_BIT(RCC->CFGR,RCC_CFGR_PPRE2);
  SET_BIT(RCC->CFGR,(APB2_Prescaler_<<RCC_CFGR_PPRE2_Pos));

  /**********************Systick config**********************/
  SystemCoreClock = RCC_GetSysClockFreq();
  // Configure the SysTick to have interrupt in 1ms time basis
  SysTick_Config((SystemCoreClock / (1000U / uwTickFreq)) > 0U);
  prioritygroup = 0x00U;
  prioritygroup = NVIC_GetPriorityGrouping();
  NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(prioritygroup, 0U, 0U));
  uwTickPrio = 0U;

  /***************************GPIOB section****************************/
  //GPIOB Reset
  SET_BIT(RCC->AHB1RSTR,RCC_AHB1RSTR_GPIOBRST);
  CLEAR_BIT(RCC->AHB1RSTR,RCC_AHB1RSTR_GPIOBRST);

  //GPIOB clock enable
  SET_BIT(RCC->AHB1ENR,RCC_AHB1ENR_GPIOBEN);

  //GPIOB Moder
  SET_BIT(GPIOB->MODER,GPIO_MODER_MODE14_0);
  SET_BIT(GPIOB->MODER,GPIO_MODER_MODE7_0);

  //GPIOB OSPEEDR
  SET_BIT(GPIOB->OSPEEDR,GPIO_OSPEEDR_OSPEED14_0);
  SET_BIT(GPIOB->OSPEEDR,GPIO_OSPEEDR_OSPEED7_0);

  //GPIOB BSRR
  SET_BIT(GPIOB->BSRR,GPIO_BSRR_BS14);
  SET_BIT(GPIOB->BSRR,GPIO_BSRR_BS7);

  /*****************************GPIOF section*****************************/
  //GPIOF Reset
  SET_BIT(RCC->AHB1RSTR,RCC_AHB1RSTR_GPIOFRST);
  CLEAR_BIT(RCC->AHB1RSTR,RCC_AHB1RSTR_GPIOFRST);

  //GPIOF clock enable
  SET_BIT(RCC->AHB1ENR,RCC_AHB1ENR_GPIOFEN);

  //GPIOF Moder
  SET_BIT(GPIOF->MODER,GPIO_MODER_MODE12_0);

  //GPIOF OSPEEDR
  SET_BIT(GPIOF->OSPEEDR,GPIO_OSPEEDR_OSPEED12);

  /*****************************SPI1 section******************************/
  //GPIOA setup for SPI1(PA5,PA6,PA7)

  //GPIOA Reset
  SET_BIT(RCC->AHB1RSTR,RCC_AHB1RSTR_GPIOARST);
  CLEAR_BIT(RCC->AHB1RSTR,RCC_AHB1RSTR_GPIOARST);

  //GPIOA clock enable
  SET_BIT(RCC->AHB1ENR,RCC_AHB1ENR_GPIOAEN);

  //GPIOA Moder
  SET_BIT(GPIOA->MODER,GPIO_MODER_MODE5_1);
  SET_BIT(GPIOA->MODER,GPIO_MODER_MODE6_1);
  SET_BIT(GPIOA->MODER,GPIO_MODER_MODE7_1);

  //GPIOA OSPEEDR
  SET_BIT(GPIOA->OSPEEDR,GPIO_OSPEEDR_OSPEED5);
  SET_BIT(GPIOA->OSPEEDR,GPIO_OSPEEDR_OSPEED6);
  SET_BIT(GPIOA->OSPEEDR,GPIO_OSPEEDR_OSPEED7);

  //GPIOA AFRL
  SET_BIT(GPIOA->AFR[0],GPIO_AFRL_AFRL5_0 | GPIO_AFRL_AFSEL5_2);
  SET_BIT(GPIOA->AFR[0],GPIO_AFRL_AFRL6_0 | GPIO_AFRL_AFSEL6_2);
  SET_BIT(GPIOA->AFR[0],GPIO_AFRL_AFRL7_0 | GPIO_AFRL_AFSEL7_2);


  //SPI1 RESET
  SET_BIT(RCC->APB2RSTR,RCC_APB2RSTR_SPI1RST);
  CLEAR_BIT(RCC->APB2RSTR,RCC_APB2RSTR_SPI1RST);

  //SPI1 clock Enable
  SET_BIT(RCC->APB2ENR,RCC_APB2ENR_SPI1EN);

  //Disable Peripheral
  CLEAR_BIT(SPI1->CR1,SPI_CR1_SPE);

  //SPI_CR1 setup
  CLEAR_BIT(SPI1->CR1,SPI_CR1_CPOL | SPI_CR1_CPHA);  //cpha = 1st edge, cpol = 0
  SET_BIT(SPI1->CR1,SPI_CR1_MSTR | SPI_CR1_SSI);  //master mode
  CLEAR_BIT(SPI1->CR1,SPI_CR1_DFF);   //8bit mode
  CLEAR_BIT(SPI1->CR1,SPI_CR1_BIDIMODE);  //2line direction
  SET_BIT(SPI1->CR1,SPI_CR1_SSM);   //NSS software slave management
  SET_BIT(SPI1->CR1,SPI_CR1_BR_0 | SPI_CR1_BR_2);   //fclk/64
  CLEAR_BIT(SPI1->CR1,SPI_CR1_LSBFIRST); //MSB first
  SET_BIT(SPI1->CR1,SPI_CR1_CRCEN); //enable hardware CRC calculation

  SET_BIT(SPI1->CRCPR,(SPI_CRC_POLY_<<0U)); //spi crc polynomial

  CLEAR_BIT(SPI1->I2SCFGR,SPI_I2SCFGR_I2SMOD);  //select spi mode

  /*************************Timer setup**********************/
  //Enable Timer
  SET_BIT(RCC->APB1ENR,RCC_APB1ENR_TIM7EN);
  NVIC_SetPriority(TIM7_IRQn,0x13);
  NVIC_EnableIRQ(TIM7_IRQn);

  // Start by making sure the timer's 'counter' is off.
  CLEAR_BIT(TIM7->CR1,TIM_CR1_CEN);

  //Timer Reset
  SET_BIT(RCC->APB1RSTR,RCC_APB1RSTR_TIM7RST);
  CLEAR_BIT(RCC->APB1RSTR,RCC_APB1RSTR_TIM7RST);

  //Set Prescaler (APB1 CLK - 84MHz)
  SET_BIT(TIM7->PSC,(TIM7_PSC_<<0U));

  //Set ARR
  CLEAR_BIT(TIM7->ARR,TIM_ARR_ARR);
  SET_BIT(TIM7->ARR,(TIM7_ARR_<<0U));

  //Auto-Reload Preload
//  SET_BIT(TIM7->CR1,TIM_CR1_ARPE);
  CLEAR_BIT(TIM7->CR1,TIM_CR1_OPM);      //Continuous timer

  //Send an update event to reset the timer and apply settings.
  SET_BIT(TIM7->EGR,TIM_EGR_UG);

  //Enable Interrupt
  SET_BIT(TIM7->DIER,TIM_DIER_UIE);
//  for(int i=0;i<10;i++);

  //Enable Counter
  SET_BIT(TIM7->CR1,TIM_CR1_CEN);

  /* USER CODE END 1 */

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* USER CODE BEGIN 2 */

  LL_GPIO_SetOutputPin(SPI1_CS_GPIO_Port, SPI1_CS_Pin); //initialize slave selecet pin as HIGH
  handle_command_only_subcommands(BQ769x2_RESET); // reset BQ769x2 chip
  delay_ms(600);
  BQ769x2_Init();  // Configure all of the BQ769x2 register settings
  delay_ms(100);
  handle_command_only_subcommands(FET_ENABLE); // Enable the CHG and DSG FETs
  delay_ms(100);
  handle_command_only_subcommands(SLEEP_DISABLE); // Sleep mode is enabled by default. For this example, Sleep is disabled to
    									   // demonstrate full-speed measurements in Normal mode.
  delay_ms(60); delay_ms(60); delay_ms(60); delay_ms(60);  //wait to start measurements after FETs close
  BQ769x2_WriteCellBalance(0x0070);
  delay_ms(5);
  BQ769x2_WriteCellBalance(0x0070);
  delay_ms(5);
//  BQ769x2_WriteCellBalance(0x0028);
//  delay_ms(5);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  BQ769x2_ReadAllVoltages(); //Read all available voltages
	  delay_ms(5);
	  BQ769x2_ReadCellBalance();   //Read active cell being balanced

  }
  /* USER CODE END 3 */
}

/* USER CODE BEGIN 4 */
/**
 * @brief  function to provide delay in milliseconds specified by user
 * @param  ms milliseconds to wait
 * @retval none
 */
void delay_ms(uint32_t ms){
	uint64_t counter_now = counter_millis;
	while((counter_millis-counter_now)<ms){
		//wait for it
	}


}

/**
 * @brief  ISR routine for TIM7
 * @param  none
 * @retval none
 * @note   check for the interrupt flag and increase counter
 */
void TIM7_IRQHandler(void)
{
	if(READ_BIT(TIM7->SR,TIM_SR_UIF)){
		CLEAR_BIT(TIM7->SR,TIM_SR_UIF); //clear the interrupt flag
		counter_millis++;
	}
}

/**
 * @brief  function to check for the current system core clock
 * @param  none
 * @retval system clock frequency
 * @note   gets system core frequency in MHz based on the clock source set
 */
uint32_t RCC_GetSysClockFreq(void)
{
  uint32_t pllm = 0U;
  uint32_t pllvco = 0U;
  uint32_t pllp = 0U;
  uint32_t pllr = 0U;
  uint32_t sysclockfreq = 0U;

  /* Get SYSCLK source -------------------------------------------------------*/
  switch (RCC->CFGR & RCC_CFGR_SWS)
  {
    case RCC_CFGR_SWS_HSI:  /* HSI used as system clock source */
    {
      sysclockfreq = HSI_VALUE;
       break;
    }
    case RCC_CFGR_SWS_HSE:  /* HSE used as system clock  source */
    {
      sysclockfreq = HSE_VALUE;
      break;
    }
    case RCC_CFGR_SWS_PLL:  /* PLL/PLLP used as system clock  source */
    {
      /* PLL_VCO = (HSE_VALUE or HSI_VALUE / PLLM) * PLLN
      SYSCLK = PLL_VCO / PLLP */
      pllm = RCC->PLLCFGR & RCC_PLLCFGR_PLLM;
      if((RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC) != RCC_PLLCFGR_PLLSRC_HSI)
      {
        /* HSE used as PLL clock source */
        pllvco = (uint32_t) ((((uint64_t) HSE_VALUE * ((uint64_t) ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> RCC_PLLCFGR_PLLN_Pos)))) / (uint64_t)pllm);
      }
      else
      {
        /* HSI used as PLL clock source */
        pllvco = (uint32_t) ((((uint64_t) HSI_VALUE * ((uint64_t) ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> RCC_PLLCFGR_PLLN_Pos)))) / (uint64_t)pllm);
      }
      pllp = ((((RCC->PLLCFGR & RCC_PLLCFGR_PLLP) >> RCC_PLLCFGR_PLLP_Pos) + 1U) *2U);

      sysclockfreq = pllvco/pllp;
      break;
    }
    case RCC_CFGR_SWS_PLLR:  /* PLL/PLLR used as system clock  source */
    {
      /* PLL_VCO = (HSE_VALUE or HSI_VALUE / PLLM) * PLLN
      SYSCLK = PLL_VCO / PLLR */
      pllm = RCC->PLLCFGR & RCC_PLLCFGR_PLLM;
      if((RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC) != RCC_PLLCFGR_PLLSRC_HSI)
      {
        /* HSE used as PLL clock source */
        pllvco = (uint32_t) ((((uint64_t) HSE_VALUE * ((uint64_t) ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> RCC_PLLCFGR_PLLN_Pos)))) / (uint64_t)pllm);
      }
      else
      {
        /* HSI used as PLL clock source */
        pllvco = (uint32_t) ((((uint64_t) HSI_VALUE * ((uint64_t) ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> RCC_PLLCFGR_PLLN_Pos)))) / (uint64_t)pllm);
      }
      pllr = ((RCC->PLLCFGR & RCC_PLLCFGR_PLLR) >> RCC_PLLCFGR_PLLR_Pos);

      sysclockfreq = pllvco/pllr;
      break;
    }
    default:
    {
      sysclockfreq = HSI_VALUE;
      break;
    }
  }
  return sysclockfreq;
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

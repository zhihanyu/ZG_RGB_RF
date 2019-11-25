#ifndef HAL_DMA_H
#define HAL_DMA_H


#define DMA_STRUCT_SIZE   (24*4)

typedef enum
{
    DMA_CH0 = 0U,
    DMA_CH1 = 1U,
    DMA_CH2 = 2U,
    DMA_CH3 = 3U,
    DMA_CH4 = 4U,
    DMA_CH5 = 5U,
    DMA_CH6 = 6U,
    DMA_CH7 = 7U,
    DMA_MAX_CH
}dma_ch_e;


typedef enum
{
  HAL_DMA_STATE_RESET             = 0x00,  /*!< DMA not yet initialized or disabled */  
  HAL_DMA_STATE_READY             = 0x01,  /*!< DMA process success and ready for use   */
  HAL_DMA_STATE_READY_HALF        = 0x11,  /*!< DMA Half process success            */
  HAL_DMA_STATE_BUSY              = 0x02,  /*!< DMA process is ongoing              */     
  HAL_DMA_STATE_TIMEOUT           = 0x03,  /*!< DMA timeout state                   */  
  HAL_DMA_STATE_ERROR             = 0x04,  /*!< DMA error state                     */                                                            
}dma_status_e;


typedef struct
{
  uint32_t dir;                		  /*!< Specifies if the data will be transferred from memory to peripheral, 
                                           from memory to memory or from peripheral to memory.
                                           This parameter can be a value of @ref DMA_Data_transfer_direction */

  uint32_t peripheral_inc;                 /*!< Specifies whether the Peripheral address register should be incremented or not.
                                           This parameter can be a value of @ref DMA_Peripheral_incremented_mode */
                               
  uint32_t memory_inc;                    /*!< Specifies whether the memory address register should be incremented or not.
                                           This parameter can be a value of @ref DMA_Memory_incremented_mode */
  
  uint32_t PeriphDataAlignment;       /*!< Specifies the Peripheral data width.
                                           This parameter can be a value of @ref DMA_Peripheral_data_size */

  uint32_t MemDataAlignment;          /*!< Specifies the Memory data width.
                                           This parameter can be a value of @ref DMA_Memory_data_size */
                               
  uint32_t Mode;                      /*!< Specifies the operation mode of the DMAy Channelx.
                                           This parameter can be a value of @ref DMA_mode
                                           @note The circular buffer mode cannot be used if the memory-to-memory
                                                 data transfer is configured on the selected Channel */ 
} dma_config_t;


typedef struct dma_hdl_
{  
  dma_ch_e   	ch;                                                   
  dma_config_t  config;                                                       
  hal_lock_e    lock;                                                   
  dma_status_e  status;                                     
  void          *parent;                                                   
  void          (* xfer_down_event)( void * hdma);   
  void          (* xfer_halfdown_event)( void * hdma);
  void          (* xfer_error_event)( void * hdma);   
  __IO uint32_t error;                                                   
} dma_hdl_t;    



















#endif


/*
  This example code is in the public domain.

  This example code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
This example is a demo of the analog-to-digital converter. 
It will show the digital value of pin ANALOG_PIN in the Monitor logs.
*/

#include "vmtype.h"
#include "vmboard.h"
#include "vmsystem.h"
#include "vmlog.h"
#include "vmdcl.h"
#include "vmdcl_gpio.h"
#include "vmdcl_adc.h"
#include "vmtimer.h"
#include "ResID.h"


#if defined(__HDK_LINKIT_ONE_V1__)
    #define ANALOG_PIN  VM_PIN_D14
#elif defined(__HDK_LINKIT_ASSIST_2502__)
    #define ANALOG_PIN  VM_PIN_P10
#else
    #error “ Board not support”
#endif

static VMUINT32 g_adc_result = 0;
static VM_DCL_HANDLE g_gpio_handle_A0 = VM_DCL_HANDLE_INVALID;
static void adc_demo_handle_sysevt(VMINT message, VMINT param);


void vm_main(void) 
{
    /* register system events handler */
    vm_pmng_register_system_event_callback(adc_demo_handle_sysevt);
}

void adc_demo_callback(void* parameter, VM_DCL_EVENT event, VM_DCL_HANDLE device_handle)
{
    vm_dcl_callback_data_t *data;
    vm_dcl_adc_measure_done_confirm_t * result;   /* The structure of ADC measure result for event VM_DCL_ADC_GET_RESULT. */
    vm_dcl_adc_control_send_stop_t stop_data;
    VMINT status = 0;

    vm_log_info("adc_demo_callback - START");
    if(parameter!=NULL)    //有数据传入
      {
        data = ( vm_dcl_callback_data_t*)parameter;//将parameter以vm_dcl_callback_data_t结构指针的方式传给data结构体
        result = (vm_dcl_adc_measure_done_confirm_t *)(data->local_parameters);

        if( result != NULL )
        {
            double *p;

            p =(double*)&(result->value);/* Indicate the measure raw data 表示原始数据 */
            g_adc_result = (unsigned int)*p;
        }
     }

    /* Stop ADC 开关之前都要先获得当前使用dcl的模块id*/
    stop_data.owner_id = vm_dcl_get_owner_id();   // Gets the identifier of the user.
    status = vm_dcl_control(g_gpio_handle_A0,VM_DCL_ADC_COMMAND_SEND_STOP,(void *)&stop_data);

    vm_log_info("adc_demo_callback,result = %d;",g_adc_result);

    vm_dcl_close(g_gpio_handle_A0);

    vm_log_info("adc_demo_callback - END");
}


static void adc_demo(void)
{
    vm_dcl_adc_control_create_object_t obj_data;   //The structure of ADC control command
    VMINT status = 0 , i;
    vm_dcl_adc_control_send_start_t start_data;    //The structure of ADC control command

    vm_log_info("adc_demo - START");

    /* Set ANALOG_PIN to mode 2 设置ADC引脚时要先开后关？*/
    g_gpio_handle_A0 = vm_dcl_open(VM_DCL_GPIO, ANALOG_PIN);// ANALOG_PIN = VM_PIN_D14
    vm_dcl_control(g_gpio_handle_A0,VM_DCL_GPIO_COMMAND_SET_MODE_2,NULL);
    vm_dcl_close(g_gpio_handle_A0);

    vm_log_info("adc_demo - set ANALOG_PIN to mode_2, handle = %d", g_gpio_handle_A0);

    /* Open ANALOG_PIN as ADC device */
    g_gpio_handle_A0 = vm_dcl_open(VM_DCL_ADC,0);

    /* register ADC result callback 重要，注册回调函数*/
    status = vm_dcl_register_callback(g_gpio_handle_A0, VM_DCL_ADC_GET_RESULT ,(vm_dcl_callback)adc_demo_callback, (void *)NULL);

    /* Indicate to the ADC module driver to notify the result. */
    obj_data.owner_id = vm_dcl_get_owner_id();
    /* Set physical ADC channel which should be measured. */
    obj_data.channel = VM_DCL_ADC_YM_CHANNEL;    //通道12
    /* Set measurement period, the unit is in ticks. */
    obj_data.period = 1;
    /* Measurement count. */
    obj_data.evaluate_count = 1;
    /* Whether to send message to owner module or not. */
    obj_data.send_message_primitive = 1;

    /* setup ADC object */
    status = vm_dcl_control(g_gpio_handle_A0,VM_DCL_ADC_COMMAND_CREATE_OBJECT,(void *)&obj_data); //重要格式，用于设置ADC，参数为句柄、命令的种类、命令结构体指针

    /* start ADC */
    start_data.owner_id = vm_dcl_get_owner_id();  /* Indicate the ADC module driver to notify the result. */
    status = vm_dcl_control(g_gpio_handle_A0,VM_DCL_ADC_COMMAND_SEND_START,(void *)&start_data);

    vm_log_info("adc_demo - END");

    return;
}

void adc_demo_handle_sysevt(VMINT message, VMINT param)
{
    switch (message)
    {
    case VM_EVENT_CREATE:
        /* delay for logs */
        vm_thread_sleep(5000);//先休眠5s
        vm_log_info("Sample of ADC - Start.");
        adc_demo();

        break;

    case VM_EVENT_QUIT:
        vm_log_info("Sample of ADC - End.");
        break;
    }
}



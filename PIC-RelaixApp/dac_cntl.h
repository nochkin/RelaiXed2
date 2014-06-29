/*********************************************************************
 * Detect whether this Relaixed board connects to a 4-input DAC
 * Control the DAC input selection together with the relaixed inputs
 ********************************************************************/
#include <stdint.h>

// values of dac_state
#define DAC_ABSENT 0
#define DAC_INACTIVE 1
#define DAC_NOLOCK 2
#define DAC_LOCKED 3

extern uint8_t dac_status( void);

/* Is a DAC connected to the Relaixed i2c bus? */
extern void dac_init(void);
extern char dac_present; // read-only for other modules

/* Choose DAC input: 0 .. 3. A value >=4 switches DAC off. */
extern void dac_set_channel( uint8_t sel);

/* regularly check state in case of NOLOCK */
extern void dac_check_lock(void);

#ifndef __CCDEBUG_CORE_H__
#define __CCDEBUG_CORE_H__

#define U16_F "4d"
#define S16_F "4d"
#define X16_F "4x"
#define U32_F "8ld"
#define S32_F "8ld"
#define X32_F "8lx"

/** lower two bits indicate debug level
 * - 0 all
 * - 1 warning
 * - 2 serious
 * - 3 severe
 */
#define CC_DBG_LEVEL_ALL     0x00
#define CC_DBG_LEVEL_OFF     CC_DBG_LEVEL_ALL /* compatibility define only */
#define CC_DBG_LEVEL_WARNING 0x01 
#define CC_DBG_LEVEL_SERIOUS 0x02 		/* memory allocation failures, ... */
#define CC_DBG_LEVEL_SEVERE  0x03
#define CC_DBG_MASK_LEVEL    0x03

/** flag for CC_DEBUGF to enable that debug message */
#define CC_DBG_ON            0x80U
/** flag for CC_DEBUGF to disable that debug message */
#define CC_DBG_OFF           0x00U

/** flag for CC_DEBUGF indicating a tracing message (to follow program flow) */
#define CC_DBG_TRACE         0x40U
/** flag for CC_DEBUGF indicating a state debug message (to follow module states) */
#define CC_DBG_STATE         0x20U
/** flag for CC_DEBUGF indicating newly added code, not thoroughly tested yet */
#define CC_DBG_FRESH         0x10U
/** flag for CC_DEBUGF to halt after printing this debug message */
#define CC_DBG_HALT          0x08U

#ifndef CC_NOASSERT
#define CC_ASSERT(message, assertion) do { if(!(assertion)) \
  CCDBG_PLATFORM_ASSERT(message); } while(0)
#else  /* CC_NOASSERT */
#define CC_ASSERT(message, assertion) 
#endif /* CC_NOASSERT */

/** if "expression" isn't true, then print "message" and execute "handler" expression */
#ifndef CC_ERROR
#define CC_ERROR(message, expression, handler) do { if (!(expression)) { \
  CCDBG_PLATFORM_ASSERT(message); handler;}} while(0)
#endif /* CC_ERROR */

#define CC_DEBUG						1
/**
 * CC_DBG_MIN_LEVEL: After masking, the value of the debug is
 * compared against this value. If it is smaller, then debugging
 * messages are written.
 */
#ifndef CC_DBG_MIN_LEVEL
#define CC_DBG_MIN_LEVEL              	CC_DBG_LEVEL_ALL
#endif

#ifdef CC_DEBUG
/** print debug message only if debug message type is enabled...
 *  AND is of correct type AND is at least CC_DBG_LEVEL
 */
#define CC_DEBUGF(debug, message) do { \
                               if ( \
                                   ((debug) & CC_DBG_ON) && \
                                   ((short)((debug) & CC_DBG_MASK_LEVEL) >= CC_DBG_MIN_LEVEL)) { \
                                 CCDBG_PLATFORM_DIAG(message); \
                                 if ((debug) & CC_DBG_HALT) { \
                                   while(1); \
                                 } \
                               } \
                             } while(0)

#else  /* CC_DEBUG */
#define CC_DEBUGF(debug, message) 
#endif /* CC_DEBUG */

#endif


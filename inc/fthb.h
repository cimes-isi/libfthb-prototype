/**
 * A prototype heartbeat interface for HPSC fault tolerance.
 *
 * @author Connor Imes
 * @date 2018-08-08
 */
#ifndef _FTHB_H_
#define _FTHB_H_

#ifdef __cplusplus
extern "C" {
#endif

// An opaque context struct
typedef struct fthb fthb;

/**
 * Create a heartbeat instance.
 *
 * @param uid A unique identifier
 * @param timeout The timeout period (units not specified)
 * @return A heartbeat pointer on success, NULL on error
 */
fthb* fthb_create(int uid, unsigned int timeout);

/**
 * Destroy a heartbeat instance.
 *
 * @param hb A heartbeat pointer
 * @return 0 on success, a negative value on error
 */
int fthb_destroy(fthb* hb);

/**
 * Issue a heartbeat pulse.
 *
 * @param hb A heartbeat pointer
 * @return 0 on success, a negative value on error
 */
int fthb_issue(fthb* hb);

/**
 * Get a heartbeat by UID.
 *
 * @param uid The unique identifier
 * @return A heartbeat pointer on success, NULL on error
 */
fthb* fthb_get_hb(int uid);

/**
 * Put (Return/detach) a heartbeat acquired with `fthb_get_hb`.
 *
 * @param hb A heartbeat pointer
 */
void fthb_put_hb(fthb* hb);

/**
 * Get the heartbeat timeout.
 *
 * @param hb A heartbeat pointer
 * @return The timeout value
 */
unsigned int fthb_get_timeout(const fthb* hb);

/**
 * Read the heartbeat counter.
 *
 * @param hb A heartbeat pointer
 * @return The most recent counter value
 */
unsigned int fthb_read_counter(fthb* hb);

//
// The following section is a test in managing heartbeats opaquely
//

/**
 * Create an opaque heartbeat instance.
 *
 * @param uid A unique identifier
 * @param uid The timeout period (unit is not specified)
 * @return 0 on success, a negative value on error
 */
int fthb_create_opaque(int uid, unsigned int timeout);

/**
 * Destroy an opaque heartbeat instance.
 *
 * @param uid A unique identifier
 * @return 0 on success, a negative value on error
 */
int fthb_destroy_opaque(int uid);

/**
 * Issue an opaque heartbeat pulse.
 *
 * @param uid A unique identifier
 * @return 0 on success, a negative value on error
 */
int fthb_issue_opaque(int uid);

#ifdef __cplusplus
}
#endif

#endif

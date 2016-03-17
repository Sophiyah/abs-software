#ifndef __SDB_PRIVATE_H
#define __SDB_PRIVATE_H

#include <stdbool.h>
#include <pthread.h>
#include <sdb.h>
#include <mcs.h>

#define SDB_MODULE_DATA_SIZE    1024

#define SDB_MODULE_MAX          20
#define SDB_USB_ID              (SDB_MODULE_MAX + 1)

typedef struct SDBPacket {
    unsigned int id_process;
    MCSPacket *pkt;
} SDBPacket;

typedef struct SDBQueueElem {
    SDBPacket *pkt;
    struct SDBQueueElem *next;
} SDBQueueElem;

typedef struct SDBQueue {
    SDBQueueElem *queue_first;
    SDBQueueElem *queue_last;
} SDBQueue;

typedef struct SDBModule {
    unsigned int id;
    pthread_t thread_id;
    int rfd; /* The SDB reads from this File Descriptor */
    int wfd; /* The SDB writes to this File Descriptor */
    const char *name;
    SDBGroup group;

    pthread_mutex_t lock;
    pthread_cond_t cond_var;

    void *data;
    bool data_valid;

    bool data_socket;

    SDBQueue queue;
} SDBModule;

extern SDBModule sdb_module[SDB_MODULE_MAX];
extern unsigned int sdb_module_last;
extern pthread_mutex_t sdb_module_lock;
extern pthread_key_t sdb_module_info;
extern int sdb_observer_fd;

/* SDB module section */
void sdb_module_init(int rfd, int wfd);
int sdb_module_write_mcs_packet(const MCSPacket *pkt, unsigned int to);
MCSPacket *sdb_module_read_mcs_packet(void);
void sdb_module_cancel_all(void);
void *sdb_module_thread(void *arg);

/* SDB director section */
void *sdb_director_thread();

/* SDB observer section */
void *sdb_observer_thread();
void sdb_observer_wake_up(void);

/* SDB queue section */
SDBPacket *sdb_packet(MCSPacket *pkt, unsigned int id);
void sdb_packet_free(SDBPacket *sdb_pkt);
void sdb_queue_push(SDBQueue *queue, SDBPacket *sdb_pkt);
SDBPacket *sdb_queue_get(SDBQueue *queue, MCSPacket *answer);

#endif

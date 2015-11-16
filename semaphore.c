#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

typedef struct m_sem_t {
  int value;
  pthread_mutex_t lock;
  pthread_cond_t notify;
} m_sem_t;

void sem_init(m_sem_t *s, int value);
int sem_wait(m_sem_t *s);
int sem_post(m_sem_t *s);

void sem_init(m_sem_t *s, int value){
  s->value = value;
  pthread_mutex_init(&s->lock,NULL);
  pthread_cond_init(&s->notify,NULL);
}
int sem_wait(m_sem_t *s)
{
    //TODO
    pthread_mutex_lock(&s->lock);
    while(s->value == 0){
      pthread_cond_wait(&s->notify,&s->lock);
    }
    s->value = 0;
    pthread_mutex_unlock(&s->lock);
    return 0;
}

int sem_post(m_sem_t *s)
{
    //TODO
    pthread_mutex_lock(&s->lock);
    s->value=1;
    pthread_cond_signal(&s->notify);
    pthread_mutex_unlock(&s->lock);
    return 0;
}

/*
 * Philosopher Noodle Problem
 * With One Philosopher Getting Priority
 * Alyxandra Spikerman
 * High Performace Computing
 * Jan 2019
 * Question 1 Homework 2
 */
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstdint>

using namespace std;

// GLOBAL VARIABLES
// later, we will initialize all the arrays to be the length of PHILOSOPHERS
int PHILOSOPHERS;
pthread_mutex_t* locks;
sem_t* sems;
int* p_eaten; // keep track of which philosophers have eaten
int *p_thought; // keep track of which philosophers have thought

/*
 * get_next_sem:
 * - next semaphore is the current semaphore + 1
 *
 * @current_sem: current semaphore
 *
 */
int get_next_sem(int current_sem) {
    int next_sem = current_sem + 1;
    if (current_sem == PHILOSOPHERS - 1) {
        next_sem = 0; // last semaphores next sem is the first sem
    }
    return next_sem;
}

/*
 * get_left_fork:
 * - left fork = thread_num - 1
 *
 * @p_num: thread number (philosopher number)
 *
 */
int get_left_fork(int p_num) {
    int left;
    if (p_num == 0) {
        left = PHILOSOPHERS - 1; // the first philosopher's left fork is the last fork
    } else {
        left = p_num - 1;
    }
    return left;
}

/*
 * get_right_fork:
 * - right fork = current thread num
 *
 * @p_num: thread number (philosopher number)
 *
 */
int get_right_fork(int p_num) {
    return p_num;
}

/*
 * all_philosophers_done:
 * - checks if all the philosophers have both eaten and thought
 *
 */
bool all_philosophers_done() {
    // initialize have_eaten and have_thought to true, set them later to false if one philosopher has not done it
    bool have_eaten = true;
    bool have_thought = true;
    for (int i = 0; i < PHILOSOPHERS; i++) {
        if (!p_eaten[i]) {
            have_eaten = false;
        }
        if (!p_thought[i]) {
            have_thought = false;
        }
    }
    return have_eaten && have_thought;
}

/*
 * print_state:
 * - prints the state of the current philosopher by waiting for the last philosopher to
 *   print their state
 *
 * @thread_num: integer value for thread/philosopher number
 * @state: state of the philosopher, either 'thinking' or 'eating'
 * @action: action of the philosopher, either 'has' or 'waiting for'
 *
 */
void print_state(int thread_num, string state, string action) {
    if (!all_philosophers_done()) { // check if all the philosophers are done or not, or else we output bad data
        sem_wait(&sems[thread_num]); // wait until this semaphore is signaled
        if (thread_num == 0) {
            cout << "\n\n-----TABLE STATE-----\n\n";
        }
        cout << "Philosopher " << thread_num << " " << state << endl;
        cout << "- " << action << " left fork: " << get_left_fork(thread_num) << endl;
        cout << "- " << action << " right fork: " << get_right_fork(thread_num) << endl << endl;
        sem_post(&sems[get_next_sem(thread_num)]); // signal the next semaphore
    }
    return;
}

/*
 * let_them_eat:
 * - takes in the thread number and tries to get a left and right fork to eat
 * - otherwise, the philosopher thinks
 * - the philosopher can only have either 2 forks in hand or no forks in hand
 *   (they do not hold a fork if they can't get the second one)
 *
 * @arg : void pointer to for the thread number
 *
 */
void* let_them_eat(void *arg) {
    uint64_t thread_num = (uint64_t)arg;

    while (!all_philosophers_done()) {
        // try to get the left and right fork to eat, else think
        if (pthread_mutex_trylock(&locks[get_left_fork(thread_num)]) == 0) {
            if (pthread_mutex_trylock(&locks[get_right_fork(thread_num)]) == 0) {
                print_state(thread_num, "eating", "has");
                p_eaten[thread_num] = 1;

                sleep(1); // simulate some eating time
                pthread_mutex_unlock(&locks[get_right_fork(thread_num)]);
            } else {
                print_state(thread_num, "thinking", "waiting for");
                p_thought[thread_num] = 1;
            }
            pthread_mutex_unlock(&locks[get_left_fork(thread_num)]);
        } else {
            print_state(thread_num, "thinking", "waiting for");
            p_thought[thread_num] = 1;
        }
        sleep(1); // let the thread pause so all the threads aren't pushing for the same mutexes at the same moment
    }
    // prevent weird output and stalling by waiting a second and signaling to the next semaphore
    sleep(1);
    sem_post(&sems[get_next_sem(thread_num)]); // signal the next semaphore
    return NULL;
}

/*
 */
int main(int argc, char* argv[]) {
    // get the command line arguments and error out if there aren't enough
    if (argc == 2) {
        PHILOSOPHERS = atoi(argv[1]);
    } else {
        cout << "Error: must input 1 arguments, <number of philosophers>" << endl;
        return 1;
    }

    // assign array sizes to all of our global arrays
    // each thread gets a lock, a semaphore, a p_eaten (1 true or 0 false) and a p_thought (1 true or 0 false)
    locks = new pthread_mutex_t[PHILOSOPHERS];
    sems = new sem_t[PHILOSOPHERS];
    p_eaten = new int[PHILOSOPHERS];
    p_thought = new int[PHILOSOPHERS];

    // mutex scheduling priority attribute
    // source https://forum.juce.com/t/thread-priority-and-inheritance-mutex/5355
    pthread_mutexattr_t lock_attr;
    pthread_mutexattr_init(&lock_attr);
    pthread_mutexattr_setprotocol(&lock_attr, PTHREAD_PRIO_INHERIT); // prioritizes the thread with highest priority

    int sem_init_val; // the value to initialize the semaphores

    // intialize the mutexes and semaphores
    for (int i = 0; i < PHILOSOPHERS; i++) {
        p_eaten[i] = 0; // this philosopher has not eaten
        p_thought[i] = 0; // this philosopher has not thought

        pthread_mutex_init(&locks[i], &lock_attr);

        // if (pthread_mutex_init(&locks[i], NULL) != 0) {
        //     cout << "Error: failed to initialize mutex" << endl;
        //     return 1;
        // }
        if (i == 0) {
            sem_init_val = 1; // we want the first thread to print first
        } else {
            sem_init_val = 0;
        }
        sem_init(&sems[i], 0, sem_init_val);
    }

    // declare pthread variables
    pthread_t threads[PHILOSOPHERS];
    uint64_t args[PHILOSOPHERS];
    int pthread_return;

    // priority declarations
    pthread_attr_t thread_attr;
    sched_param s_param;

    for (int i = 0; i < PHILOSOPHERS; i++) {
        // create the thread and pass in the thread number
        args[i] = i;
        if (i == 0) {
            // give the first thread priority
            // source: https://docs.oracle.com/cd/E19455-01/806-5257/attrib-16/index.html
            pthread_attr_init(&thread_attr);
            pthread_attr_getschedparam(&thread_attr, &s_param);
            s_param.sched_priority = 20;
            pthread_attr_setschedparam(&thread_attr, &s_param);
            pthread_return = pthread_create(&threads[i], &thread_attr, let_them_eat, (void *)args[i]);
        } else {
            pthread_return = pthread_create(&threads[i], NULL, let_them_eat, (void *)args[i]);
        }
        if (pthread_return) {
            cout << "Error: cannot create thread, " << pthread_return << endl;
            return 1;
        }
    }

    for (int i = 0; i < PHILOSOPHERS; i++) {
        pthread_return = pthread_join(threads[i], NULL); // join the threads
        if (pthread_return) {
            cout << "Error: cannot join thread, " << pthread_return << endl;
            return 1;
        }
    }

    // destroy the mutexes and semaphores
    for (int i = 0; i < PHILOSOPHERS; i++) {
        pthread_mutex_destroy(&locks[i]);
        sem_destroy(&sems[i]);
    }

    return 0;
}

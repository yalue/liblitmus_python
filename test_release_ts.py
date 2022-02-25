# This script simply tests spawning several tasks and releasing them all at
# once.
import multiprocessing
import random
import time
import liblitmus_helper as liblitmus

def rt_task():
    """ This function is run by each task in a new process. """
    liblitmus.set_rt_task_param(
        exec_cost = 0.5,
        period = 1.0,
        relative_deadline = 1.0)
    liblitmus.init_litmus()
    liblitmus.task_mode(True)
    # We're not going to actually run any jobs, we're just sleeping between 0
    # and 5 seconds before waiting for the task system to "release".
    time.sleep(random.random() * 5.0)
    print("Task TID %d waiting." % (liblitmus.get_tid(),))
    liblitmus.wait_for_ts_release()
    print("Task TID %d released." % (liblitmus.get_tid(),))
    liblitmus.exit_litmus()

def run_processes():
    process_count = 10
    procs = []
    # Kick off the child processes.
    for i in range(process_count):
        p = multiprocessing.Process(target=rt_task)
        p.start()
        procs.append(p)
    # Wait for *all* of the child processes to get to wait_for_ts_release
    print("Waiting for %d tasks to be ready." % (process_count,))
    waiting = liblitmus.get_nr_ts_release_waiters()
    while waiting != process_count:
        time.sleep(0.01)
        waiting = liblitmus.get_nr_ts_release_waiters()
    liblitmus.release_ts(liblitmus.litmus_clock())
    print("Tasks released")

run_processes()


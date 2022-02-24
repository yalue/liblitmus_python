# This script simply sets up a basic real-time task that exercises the basic
# functionality exposed in liblitmus_helper.
import argparse
import random
import time
import liblitmus_helper as liblitmus

def lock_slot():
    return liblitmus.get_k_exclusion_slot()

class TestState:
    """ Keeps track of LITMUS-related state. Has GLOBAL side effects. Not much
    to do about that; since we are a real-time task after all. """

    def __init__(self, period, max_cost):
        """ Sets up LITMUS-RT stuff, and opens a K-exclusion lock. Requires
        a period and WCET in milliseconds. """
        self.lock_od = -1
        self.period = period
        self.max_cost = max_cost
        liblitmus.set_rt_task_param(
            exec_cost = self.max_cost,
            period = self.period,
            relative_deadline = self.period)
        liblitmus.init_litmus()
        liblitmus.task_mode(True)
        self.lock_od = liblitmus.open_kfmlp_lock(".kfmlp_lock", 1, 3)

    def run_jobs(self, count):
        """ Runs the given number of jobs. """
        start_time = liblitmus.litmus_clock()
        for i in range(count):
            to_sleep = random.random() * self.max_cost * 0.75
            print("At %f s: TID %d, job %d. Sleeping for %.03f s." % (
                liblitmus.litmus_clock() - start_time,
                liblitmus.get_tid(), liblitmus.get_job_no(), to_sleep))
            liblitmus.litmus_lock(self.lock_od)
            slot = lock_slot()
            print("Got lock. In slot " + str(slot))
            time.sleep(to_sleep)
            liblitmus.litmus_unlock(self.lock_od)
            print("Released lock. Was slot " + str(slot))
            liblitmus.sleep_next_period()

def run():
    parser = argparse.ArgumentParser()
    parser.add_argument("--period", "-p", type=float, default=0.1,
        help="The task's period, in seconds.")
    parser.add_argument("--wcet", "-c", type=float, default=0.05,
        help="The task's cost, in seconds.")
    parser.add_argument("--sleep_time", "-s", type=float, default=-1,
        help="The amount of seconds the task will sleep when holding the " +
            "lock. If 0 or negative, this will be a random amount between 0 " +
            "and the task's cost.")
    parser.add_argument("--jobs", "-j", type=int, default=1,
        help="The number of jobs to run.")
    parser.add_argument("--wait", action="store_true",
        help="If set, wait for the task system to be released.")
    args = parser.parse_args()
    if args.period <= 0:
        print("The task's period must be positive.")
        exit(1)
    if (args.wcet <= 0) or (args.wcet > args.period):
        print("The task's cost must be positive, but less than its period.")
        exit(1)
    if args.sleep_time >= args.wcet:
        print("The task's sleep time must be less than its cost.")
        exit(1)
    if args.jobs <= 0:
        print("The number of jobs to run must be positive.")
    state = TestState(args.period, args.wcet)
    print("LITMUS task param: %s" % (str(liblitmus.get_rt_task_param()),))
    print("LITMUS ctrl page: %s" % (str(liblitmus.get_ctrl_page()),))
    print("Current LITMUS clock: %f" % (liblitmus.litmus_clock(),))
    if args.wait:
        print("Waiting for task system release.")
        liblitmus.wait_for_ts_release()
    print("About to run jobs. TID=%d" % (liblitmus.get_tid(),))
    state.run_jobs(args.jobs)
    liblitmus.exit_litmus()

if __name__ == "__main__":
    run()


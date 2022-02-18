import liblitmus_helper as liblitmus

# TODO: Write a test case for liblitmus_helper
# See the following C code for setting up an RT task.
# if (be_migrate_to_domain(_rt_core) < 0) {
#     perror("Unable to migrate to specified CPU");
#     exit(1);
# }
# struct rt_task rt_param;
# init_rt_task_param(&rt_param);
# // Fake exec cost - this value ignored by the MC^2 scheduler
# rt_param.exec_cost = _rt_period;
# rt_param.period = _rt_period;
# rt_param.relative_deadline = 0;
# rt_param.phase = 0;
# rt_param.priority = LITMUS_LOWEST_PRIORITY;
# rt_param.cls = _rt_crit;
# rt_param.budget_policy = NO_ENFORCEMENT;
# rt_param.cpu = _rt_core;
# rt_param.release_policy = TASK_PERIODIC;
# if (set_rt_task_param(gettid(), &rt_param) < 0) {
#     perror("Unable to set real-time parameters");
#     exit(1);
# }
# if (init_litmus() != 0) {
#     perror("init_litmus failed");
#     exit(1);
# }
# if (task_mode(LITMUS_RT_TASK) != 0) {
#     perror("Unable to become real-time task");
#     exit(1);
# }
# _rt_cp = get_ctrl_page();
# if (wait && wait_for_ts_release() != 0) {
#     perror("Unable to wait for taskset release");
#     exit(1);
# }


#!/bin/bash
# The basic test I ran to test lock contention. Make sure the scheduler is set
# to GSN-EDF and the liblitmus_helper library is installed prior to running
# this.

for i in {0..12}
do
	python test_library.py --jobs 10 --wait &
done

# Sleep a bit to give the scripts time to get to wait_for_ts_release()
sleep 5
../liblitmus/release_ts

# Wait for all the scripts to finish running.
wait


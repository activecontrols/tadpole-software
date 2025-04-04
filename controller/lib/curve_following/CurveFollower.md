## CurveFollower

Contains logic for following angle and thrust curves in open loop and closed loop modes.

Router Cmds:
- follow_curve - runs the currently loaded curve
- auto_seq - loads AUTOCUR and runs, creating log files as needed

## CurveLogger

Contains logic for writing .csv logs during curve following.

## Safety

Contains logic for aborting curve following if a fault is detected.

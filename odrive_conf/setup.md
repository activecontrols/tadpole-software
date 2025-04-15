## To Setup / Restore Odrive Configs

| ODrive | CAN ID | Serial #     |
| ------ | ------ | ------------ |
| LOX    | 1      | 395E354D3231 |
| IPA    | 2      | 393F35393231 |

```cmd
.venv/Scripts/activate
odrivetool --serial-number 395E354D3231 restore-config odrive_conf/lox.json
odrivetool --serial-number 393F35393231 restore-config odrive_conf/ipa.json
```

This will restore odrive configs, **including saved cal settings.**
If calibration has changed, **re-run cal from the UI, and re-export the data!**
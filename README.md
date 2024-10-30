## Important Disclaimer

Before using the utility, please review the [disclaimer](DISCLAIMER.md).

Create and Edit the 'settings.properties' to configure Crystaller.
- maxMem - set max memory limit.
```properties
maxMem=2048
```

- ignoreName - set app that will be ignored by Crystaller.
```properties
ignoreName=vesktop
ignoreName=telegram-desktop
ignoreName=chrome
```

- ignoreNameCPU - set app that will be ignored by Crystaller's CPU checker.
```properties
ignoreNameCPU=github-desktop
ignoreNameCPU=telegram-desktop
ignoreNameCPU=chrome
```

- sleepBeforeTime - time in seconds (float also supported) before start (can be used for debug)
```properties
sleepBeforeTime=2
```

- sleepTime - time in seconds (float also supported) between checks.
```properties
sleepTime=0.5
```

- cpuon - boolean (can be used 'true', 'false', '1' and '0'). Turn on/off CPU checker. (UNSTABLE. NOT RECOMMENTED TO USE!)
```properties
cpuon=false
```

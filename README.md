## Important Disclaimer

Before using the utility, please review the [disclaimer](DISCLAIMER.md).

# Installation
You will also need memwatch:
```bash
git clone https://github.com/R0STUS/memwatch
cd memwatch
sudo make install
```

To build and run Crystaller:
```bash
git clone https://github.com/R0STUS/Crystaller
cd Crystaller
chmod +x install.sh
install.sh
crystaller
```

# Configuration

Create and Edit the '~/.config/crystaller/settings.properties' to configure Crystaller.
- maxMem - set max memory limit (in Mb).
```properties
maxMem=2048
```

- ignoreName - set app that will be ignored by Crystaller.
```properties
ignoreName=discord
ignoreName=chrome
```

- sleepTime - time in seconds (float also supported) between checks.
```properties
sleepTime=10.5
```

- sleepBeforeTime - time in seconds (float not supported) before start program (recommended for debug).
```properties
sleepBeforeTime=5
```

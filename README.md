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
chmod +x compile.sh
./compile.sh
./crystaller
```

# Configuration

Create and Edit the 'settings.properties' to configure Crystaller.
- maxMem - set max memory limit.
```properties
maxMem=2048
```

- ignoreName - set app that will be ignored by Crystaller.
```properties
ignoreName=vesktop
ignoreName=chrome
```

- sleepTime - time in seconds (float not supported) between checks.
```properties
sleepTime=0.5
```

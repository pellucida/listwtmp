## dumpwtmp, listwtmp

#### dumpwtmp
```
dumpwtmp [-f utmpfile] [-t type [-t type]..] [-O output_delimiter] [-a 'YYYY-MM-DD HH:MM:SS'

dumpwtmp -t user -a 2017-10-17
...
nautilus user 15185 pts/3 /3 nemo 10.11.12.13 verne.example.net 2017-10-17 10:49:49 1508197789
...

```
Dump the contents of a utmp formatted file restricted to the selected (-t)
types and after (-a) a particular time.


#### listwtmp
```
listwtmp [-f wtmpfile] [-D since_days_ago ]
                       [-W since_weeks_ago ]
                       [-H since_hours_ago ]
                       [-M since_minutes_ago ]
                       [-S since_seconds_ago ]

...
nautilus nemo pts/8 verne.example.net 10.11.12.13 2017-10-16
...

```
List the logins recorded in a utmp(x) file.

*last* varies sufficiently between platforms and releases that a simpler
program with easily parsed output has its uses.

### LICENSE
*Creative Commons CC0*
[http://creativecommons.org/publicdomain/zero/1.0/legalcode](http://creativecommons.org/publicdomain/zero/1.0/legalcode)

### AUTHOR
[James Sainsbury](mailto:toves@sdf.lonestar.org)


### Task1.a: Understanding

- Problem 1

- Problem 2: Why do you need the root privilege to run sniffex? Where does the program fail if executed without the root privilege?
```c
		dev = pcap_lookupdev(errbuf); // The program fails to find the default device on which to capture, 
                                    if executed without the root privilege
		if (dev == NULL) {
			fprintf(stderr, "Couldn't find default device: %s\n",
			    errbuf);
			exit(EXIT_FAILURE);
		}

```

- Problem 3

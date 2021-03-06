# swuniq
![Travis (.org)](https://img.shields.io/travis/mterron/swuniq.svg) ![coverity result](https://img.shields.io/coverity/scan/17035.svg) [![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/mterron/swuniq.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/mterron/swuniq/context:cpp)

Deduplicate matching lines (within a configurable window) from a file or standard input, writing to standard output.

Like uniq but works on unsorted input to be used as a pipe filter with constant memory usage.

#### Why?
Sometimes you need consume a data stream (Certificate Transparency log for example) that have non consecutive duplicates and you don't want to deal with them. The usual solution involving `awk` has unbounded memory usage so that might be a problem, this one doesn't.

#### Memory Usage
swuniq uses a ringbuffer of configurable size (-w option) as a FIFO queue to store hashes of each line to keep memory use constant (64bits * -w value).


#### Example
```sh
# swuniq -h
Usage: swuniq [-w N] [INPUT]
Filter matching lines (within a configurable window) from INPUT 
(or standard input), writing to standard output.

	-w N Size of the sliding window to use for deduplication
 Note: By default swuniq will use a window of 100 lines.

# cat input.txt 
apple
apple
apple
banana
banana
strawberry
blueberry
apple
banana
strawberry
blueberry
kiwifruit
orange
peach
watermelon
orange
watermelon
kiwifruit
banana
banana
banana
apple
kiwifruit

# swuniq < input.txt
apple
banana
strawberry
blueberry
kiwifruit
orange
peach
watermelon

# swuniq -w 4 < input.txt
apple
banana
strawberry
blueberry
kiwifruit
orange
peach
watermelon
banana
apple
kiwifruit

# swuniq -w 2 < input.txt 
apple
banana
strawberry
blueberry
apple
banana
strawberry
blueberry
kiwifruit
orange
peach
watermelon
orange
kiwifruit
banana
apple
kiwifruit
 
```

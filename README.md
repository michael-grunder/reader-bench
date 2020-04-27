### Clone, build, and benchmark

```bash
git clone https://github.com/michael-grunder/reader-bench --recurse-submodules
cd reader-bench

# Builds benchmarking binaries against different hiredis branches
./build.sh

# Create a file of raw multibulk protocol data
php resp-protogen/proto.php --count 1000000 --type multibulk --bulkleaf rand >| /tmp/mbk.1m.proto

# Compare raw reader performance of master and unlimited-depth branches
./raw-bench-master /tmp/mbk.1m.proto
./raw-bench-unlimited-depth /tmp/mbk.1m.proto
```

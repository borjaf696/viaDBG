# viaDBG 1.0
## Inference of viral quasispecies multi-assembler
### Purpose:
Given a set of paired_end reads from a viral sample, it returns the largest independent contigs that are contained into the sample. The entry is expected to have:
	* High coverage.
	* Genomes should be almost full covered.

The code accepts both fasta and fastq files.
### Overview:
1. Correct the reads following LorDEC's approach (optional)
	* Count k-mers (DSK is the default and the recommended way)
	* Build/Prune(soft) de Bruijn Graph from solid k-mers (k-mers above a threshold).
	* Find paths using de Bruijn Graph built between reads following solid k-mers.
	* Build a Path Graph for each read.
	* Find the shortest path (using Dijkstra with prior queue so far)
	* Repeat the process "n" times with differents k-mer size (K -> 2*K -> 2*2*K, right now)

2. Representative nodes.
	* Each unitig, arbritrarily long, is identify only by the three nodes: first, last and middle one.

3. Own version of the Approximate Paired de Bruijn Graph:
	* Removing duplicated reads.
	* Adding paired_end information.
	* Polishing paired_end information (optional) 
	* Building cliques for each pair of adjacent nodes.
	* Split and merging the nodes of the graph.
	* Reporting unitigs from the new representation.

## Tips:
1. Error and polishing are only useful when data is extremely complex. Furthermore, both have a huge efficiency impact, thus they are only recommended when result with the regular execution is far from being good. Otherwise, we encourage to use the regular execution.

2. Estimated error is the trickiest part (next versions will avoid the need of using this parameter):
	* If error is much higher the graph will not have enough nodes (solid information) and probably it will erase much more information than needed.
	* If error is much lower the graph will have too much nodes (fake solid information) and the result contigs will not be as good as you expect.
	* Current version does not use any error estimation! This problem has been overcome!
	
### Input:
1. If pear:
	* -s single-end-reads.fasta/fastq and -p paired-end-dir/
2. No-pear:
	* -p paired-end-dir/
3. Full information:
	* If true - both reverse complementary and forward reads are going to be added as paired-end information. Only recommended for complex dataset, more is not always better.
	* If not true - only forward information is added. Meaning forward as the original shape for the reads.
	* Default: False
4. Remove duplicates:
	* Ideally it makes no changes over the results. Unfortunately, it can improve or get worse, as before more is not always better. Nevertheless, speed is improved when removing duplicated reads.
5. K-mer size:
	* With the current dsk compilaton maximum k-mer size is 192.
	* A higher one can be used recompiling DSK and modifying Utils/scripts/dsk_script.
6. Reference + metaquastPath:
	* Allows to automatically check the results obtained from the execution by using metaquast. 
		* --unique-mapping flag is activated by default
		* \>500 bp contigs are shown
### Dependencies:
1. boostlib to build the graph, move through the file system and to show nicely the arguments.
	* cd /home && wget http://downloads.sourceforge.net/project/boost/boost/1.60.0/boost_1_60_0.tar.gz 
	* tar xfz boost_1_60_0.tar.gz 
	* rm boost_1_60_0.tar.gz 
	* cd boost_1_60_0 
	* ./bootstrap.sh --prefix=/usr/local --with-libraries=program_options,regex,filesystem,system
	* export  
	* ./b2 install 
	* cd /home 
	* rm -rf boost_1_60_0
	* export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/lib/
	* (just in case) ls /usr/local/bin/
2. SGA to remove duplicates for reporting contigs and remove duplicated reads.
	* google-sparsehash: https://github.com/justinsb/google-sparsehash.git
	* bamtools: https://github.com/pezmaster31/bamtools.git
	* https://github.com/jts/sga.git
3. (Optional) metaquast.py for testing the results.

### Dockerfile + web usage:
1. There is a dockerfile which autoinstall everything needed (still under revision): https://github.com/borjaf696/viaDBG-DockerFile
2. We are building an easy-to-use web interface to allow little tests (still working on it)

### Use:
	* make clean && make
	* ./bin/viaDBG -s [single_end_reads] -p [paired_end_reads(dir)] -o [output] -u [unitigs file output] -k [kmer size] -h [polish (default No)] -b [if error correction(default No)] -d [if revcomplement paired end reads] -r [estimated error] -n [remove duplicated] -f [add full information] -t [num threads] --reference [path to reference] --metaquastpath [path to metaquast.py file] --postprocess [remove duplicates from contigs file]

#### Example:
	* screen -L -Logfile Real/log-real perf stat -d ./bin/viaDBG -s single-end-file -p paired-end-dir/ -o Output/ -u Results_Real/UnitigsDiscovered_new-Real.gfa -k 120 -c dsk -n -t 32 --postprocess --reference reference.fasta --metaquastpath bin/metaquast.py
	* ./bin/viaDBG -p paired-end-file/ -o Output/SequenceContainer.fasta -u untiigs.fasta -k 120 -c dsk -n -t 32 --reference reference.fasta --metaquastpath bin/metaquast.py

 Obviously,***--reference*** & ***--metaquastpath*** are optional and their use is only for testing purpouses.

Funding:
This research has received funding from the European Union's Horizon 2020 research and innovation programme under the Marie Sklodowska-Curie [grant agreement No 690941]; Ministerio de Ciencia, Innovación y Universidades [TIN2016-78011-C4-1-R; TIN2016-77158-C4-3-R; FPU17/02742]; Xunta de Galicia [ED431C 2017/58; ED431G/01; IN848D-2017-2350417; IN852A 2018/14]; and from Academy of Finland [308030, 314170, and 323233].

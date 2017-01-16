#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<list>
#include"2cache.h"

using namespace std;


int main(int argc, char*argv[])
{
	list<cache*> cachelist;

	// page size, number of victim buffer, number of stream buffer

	//cache *cache11 = new cache(512, 4, 1);
	//cachelist.push_back(cache11);
	//cache *cache12 = new cache(512, 4, 2);
	//cachelist.push_back(cache12);
	cache *cache13 = new cache(512, 4, 4);
	cachelist.push_back(cache13);

	//cache *cache14 = new cache(512, 8, 1);
	//cachelist.push_back(cache14);
	//cache *cache15 = new cache(512, 8, 2);//y
	//cachelist.push_back(cache15);
	//cache *cache16 = new cache(512, 8, 4);//j
	//cachelist.push_back(cache16);

	//cache *cache17 = new cache(512, 16, 1);
	//cachelist.push_back(cache17);
	//cache *cache18 = new cache(512, 16, 2);
	//cachelist.push_back(cache18);
	//cache *cache19 = new cache(512, 16, 4);
	//cachelist.push_back(cache19);


	printf("read the trace file...\n");
	FILE *trace;

	//fopen_s(&trace, "trace-redis2.txt", "rt");
	fopen_s(&trace, "6t-final.txt", "rt");
	//fopen_s(&trace, "1t.txt", "rt");

	//fopen_s(&trace, "bzip2", "rt");

	if (!trace)
	{
		printf("Can't open the trace file!\n");
		return 0;
	}

	list<cache*> ::iterator  it = cachelist.begin();
	for (; it != cachelist.end(); it++)
	{
		if (!(*it)->initcache())
			return 0;
	}

	unsigned int type, addr;
	while (fscanf_s(trace, "%d %x\n", &type, &addr) != EOF)
	{
		it = cachelist.begin();
		if ((*it)->getaccessnum()%1000000==0)//till 51
			printf("%d\n", (*it)->getaccessnum());

		switch (type)
		{
		case 0:
			for (; it != cachelist.end(); it++)
			if (!(*it)->cacheread(addr))
				return 0;
			break;

		case 1:
			for (; it != cachelist.end(); it++)
			if (!(*it)->cachewrite(addr))
				return 0;
			break;

		case 2:
			for (; it != cachelist.end(); it++)
			if (!(*it)->instread(addr))
				return 0;
			break;

		case 3:
			for (; it != cachelist.end(); it++)
			if (!(*it)->cachewrite(addr))
				return 0;
			break;

		default:
			perror("invalid type\n");
			printf("%d", type);
			break;
		}
	}
	


	FILE *file;
		fopen_s(&file, "result5.csv", "wt");



	if (!file)
	{
		printf("Can't create the result1 file!\n");

		return 0;
	}

	fprintf(
		file,
		"\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\"\n",

		"victimBuffer", "streamBuffer",
		"victimBufferWrite", "victimBufferRead", "victimBufferHit",
		"streamBufferWrite", "streamBufferRead", "streamBufferHit",

		"L1-Access", "L1-Read", "L1-Write","L1-Hit", "L1-Miss", "L1-MissRate", 
		"L2-Access", "L2-Read", "L2-Write", "L2-Hit", "L2-Miss", "L2-MissRate",

		"DRAM-Access", "DRAM-Read", "DRAM-Write", "DRAM-Hit", "DRAM-Miss", "DRAM-MissRate",
		"SSD-Access", "SSD-Read", "SSD-Write", "SSD-Hit", "SSD-Miss", "SSD-MissRate",

		"TotalAccessTime","AMAT");
	printf("11");
	list<cache*>::iterator itt = cachelist.begin();
	for (; itt != cachelist.end(); itt++)
	{
		fprintf(
			file,
		"\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%lf\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%lf\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%lf\",\"%d\",\"%d\",\"%d\",\"%d\",\"%d\",\"%lf\",\"%lf\",\"%lf\"\n",
		(*itt)->getvbuffernum(), (*itt)->getsbuffernum(),
		(*itt)->getvwritenum(), (*itt)->getvreadnum(), (*itt)->getvhitmnum(),
		(*itt)->getswritenum(), (*itt)->getsreadnum(), (*itt)->getshitmnum(),

			(*itt)->getaccessnum(), (*itt)->getreadnum(), (*itt)->getwritenum(),
			(*itt)->gethitnum(), (*itt)->getmissnum(),(*itt)->getmissrate(),
			
			(*itt)->getaccessnum2(), (*itt)->getreadnum2(), (*itt)->getwritenum2(),
			(*itt)->gethitnum2(), (*itt)->getmissnum2(), (*itt)->getmissrate2(),


			(*itt)->getaccessnumd(), (*itt)->getreadnumd(), (*itt)->getwritenumd(),
			(*itt)->gethitnumd(), (*itt)->getmissnumd(), (*itt)->getmissrated(),

			(*itt)->getaccessnums(), (*itt)->getreadnums(), (*itt)->getwritenums(),
			(*itt)->gethitnums(), (*itt)->getmissnums(), (*itt)->getmissrates(),

			
			(*itt)->totalaccesstime(), (*itt)->amat()
			);
	}
	printf("22");
	fclose(trace);
	printf("close the trace file！\n");

	fclose(file);

	printf("Done！\n");
	return 0;
}
#include"2cache.h"
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>

int log2(int n)
{
	for (int i = 0; i < 64; i++)
	{
		if ((1 << i) == n)
			return i;
	}
	return -1;
}

cache::cache(int pagesize, int vbuffernum, int sbuffernum)
{
	size_block = 64;
	num_associativity = 1;
	num_set = 1024;

	//size_block2 = 64;
	num_associativity2 = 2;
	num_set2 = 4096;

	size_cache = size_block*num_associativity*num_set;
	num_block = num_associativity*num_set;

	size_cache2 = size_block*num_associativity2*num_set2;
	num_block2 = num_associativity2*num_set2;

	size_ssd = 256 * 1024 * 1024;
	size_page = pagesize;
	num_way = size_page/size_block;
	num_pages = size_ssd/size_page;
	num_blocks= num_pages*num_way;

	size_dram = 64 * 1024 * 1024;
	num_paged = size_dram / size_page;

	scachememory = NULL;
	scachememory2 = NULL;
	sssd = NULL;
	sdram = NULL;

	num_vbuffer= vbuffernum;
	num_sbuffer = sbuffernum;

}

cache::~cache()
{
	deletecache();
}

void cache::deletecache()
{
	if (scachememory)
	{
		delete[] scachememory;
		scachememory = NULL;
	}
	if (scachememory2)
	{
		delete[] scachememory2;
		scachememory2 = NULL;
	}
	if (sssd)
	{
		delete[] sssd;
		sssd = NULL;
	}
	if (sdram)
	{
		delete[] sdram;
		sdram = NULL;
	}
	if (svictimbuffer)
	{
		delete[] svictimbuffer;
		svictimbuffer = NULL;
	}
	if (sstreambuffer)
	{
		delete[] sstreambuffer;
		sstreambuffer = NULL;
	}
}

bool cache::initcache()
{
	deletecache();
	scachememory = new struct cachememory[num_block];
	memset(scachememory, 0, sizeof(struct cachememory)*num_block);

	scachememory2 = new struct cachememory2[num_block2];
	memset(scachememory2, 0, sizeof(struct cachememory2)*num_block2);

	sssd = new struct ssd[num_blocks];
	memset(sssd, 0, sizeof(struct ssd)*num_blocks);

	sdram = new struct dram[num_paged];
	memset(sdram, 0, sizeof(struct dram)*num_paged);

	if (num_vbuffer)
	{
		svictimbuffer = new victimbuffer[num_vbuffer];
		memset(svictimbuffer, 0, sizeof(struct victimbuffer)*num_vbuffer);
	}
	if (num_sbuffer)
	{
		sstreambuffer = new streambuffer[num_sbuffer];
		memset(sstreambuffer, 0, sizeof(struct streambuffer)*num_sbuffer);
	}

	req_index = log2(num_set);
	req_offset = log2(size_block);
	req_tag = 64 - (req_index + req_offset);

	req_index2 = log2(num_set2);
	req_offset2 = log2(size_block);
	req_tag2 = 64 - (req_index2 + req_offset2);

	req_pages = log2(num_pages);
	req_offsets = log2(size_block);
	req_tags = 64 - (req_pages+req_offsets);


	reqindexmask = 0;
	reqoffsetmask = 0;

	for (unsigned int i = 0; i < 64; i++)
	{
		if (i < req_offset)
			reqoffsetmask |= (1 << i);
		else if (i < req_index + req_offset)
			reqindexmask |= (1 << i);
		else
			break;
	}

	reqindexmask2 = 0;
	reqoffsetmask2 = 0;

	for (unsigned int i = 0; i < 64; i++)
	{
		if (i < req_offset2)
			reqoffsetmask2 |= (1 << i);
		else if (i < req_index2 + req_offset2)
			reqindexmask2 |= (1 << i);
		else
			break;
	}

	reqpagemasks = 0;
	reqoffsetmasks = 0;

	for (unsigned int i = 0; i < 64; i++)
	{
		if (i < req_offset2)
			reqoffsetmask2 |= (1 << i);
		else if (i < req_pages + req_offset2)
			reqpagemasks |= (1 << i);
		else
			break;
	}

	num_access = 0;
	num_miss = 0;
	num_hit = 0;
	num_read = 0;
	num_write = 0;

	num_access2 = 0;
	num_miss2 = 0;
	num_hit2 = 0;
	num_read2 = 0;
	num_write2= 0;

	num_accesss = 0;
	num_misss = 0;
	num_hits = 0;
	num_reads= 0;
	num_writes = 0;

	num_accessd = 0;
	num_missd = 0;
	num_hitd = 0;
	num_readd = 0;
	num_writed = 0;

	num_vwrite = 0;
	num_vhit = 0;
	num_vread = 0;

	num_swrite = 0;
	num_shit = 0;
	num_sread = 0;

	return true;
}

bool cache::cacheread(unsigned int address)
{
	unsigned int way;
	unsigned int baseaddress;
	unsigned int targetaddress;

	if (!getbaseaddress(getindex(address), baseaddress))
		return false;

	num_access++;
	num_read++;

	for (way = 0; way < num_associativity; way++)
	{
		targetaddress = baseaddress + way;
		if (scachememory[targetaddress].valid == 1)
		{

			if (gettag(address) == scachememory[targetaddress].tag)
			{
				num_hit++;
				return true;
			}
		}
	}
	if (cacheread2(address))
		return true;

	if (dramread(address))
		return true;

	if (ssdread(address))
		return true;
	num_miss++;
	return loadandwriteblock(address);
}

bool cache::cacheread2(unsigned int address)
{
	unsigned int way;
	unsigned int baseaddress;
	unsigned int targetaddress;

	if (!getbaseaddress2(getindex2(address), baseaddress))
		return false;

	num_access2++;
	num_read2++;

	for (way = 0; way < num_associativity2; way++)
	{
		targetaddress = baseaddress + way;
		if (scachememory2[targetaddress].valid == 1)
		{

			if (gettag2(address) == scachememory2[targetaddress].tag)
			{
				num_hit2++;
				return true;
			}
		}
	}
	num_miss2++;
	return false;
}
bool cache::victimbuferread(unsigned int address)
{
	int i, j;
	unsigned int page;
	unsigned int tag;
	unsigned int baseaddress;
	unsigned int targetaddress;

	num_vread++;

	page = getpages(address);
	tag = gettags(address);
	if (!getbaseaddresss(page, baseaddress))
		return false;

	for (i = 0; i < num_vbuffer; i++)
	{
		if ((svictimbuffer[i].valid == 1) && (svictimbuffer[i].pageno == page))
		{
			for (j = 0; j < num_way; j++)
			{
				targetaddress = baseaddress + j;
				if (sssd[targetaddress].valid == 1)
				{
					if (sssd[targetaddress].tag == tag)
					{
						num_vhit++;
						num_hitd++;
						return true;
					}
				}
			}
		}
	}
	victimbuferwrite(address);
	return false;
	
}
bool cache::victimbuferwrite(unsigned int address)
{
	int i, j;
	unsigned int page;

	page = getpages(address);

	num_vwrite++;;

	for (i = 0; i < num_vbuffer; i++)
	{
		if ((i >= 1) && svictimbuffer[i].valid == 1)
		{
			svictimbuffer[i - 1].valid = svictimbuffer[i].valid;
			svictimbuffer[i - 1].pageno = svictimbuffer[i].pageno;
		}
		if (i == (num_vbuffer - 1))
		{
			svictimbuffer[i].valid = 1;
			svictimbuffer[i].pageno = page;
		}
	}

	return true;
}
bool cache::streambufferread(unsigned int address)
{
	int i, j;
	unsigned int page;
	unsigned int tag;
	unsigned int baseaddress;
	unsigned int targetaddress;

	num_sread++;

	page = getpages(address);
	tag = gettags(address);
	if (!getbaseaddresss(page, baseaddress))
		return false;

	for (i = 0; i < num_sbuffer; i++)
	{
		if ((sstreambuffer[i].valid == 1) && (sstreambuffer[i].pageno == page))
		{
			for (j = 0; j < num_way; j++)
			{
				targetaddress = baseaddress + j;
				if (sssd[targetaddress].valid == 1)
				{
					if (sssd[targetaddress].tag == tag)
					{
						num_shit++;
						num_hitd++;
						return true;
					}
				}
			}
		}
	}
	streambufferwrite(address);
	return false;
}
bool cache::streambufferwrite(unsigned int address)
{
	int i, j;
	unsigned int page;

	page = getpages(address);

	num_swrite++;;
	
	for (i = 0; i < num_sbuffer; i++)
	{
		page++;
		sstreambuffer[i].pageno = page;
		sstreambuffer[i].valid = 1;
	}
	
	return true;
}

bool cache::dramread(unsigned int address)
{
	int i,j;
	unsigned int page;
	unsigned int tag;
	unsigned int baseaddress;
	unsigned int targetaddress;

	page = getpages(address);
	tag = gettags(address);
	if (!getbaseaddresss(page, baseaddress))
		return false;

	num_accessd++;
	num_readd++;

	if (num_sbuffer)
	if (streambufferread(address))
		return true;

	if (num_vbuffer)
	if (victimbuferread(address))
		return true;

	for (i = 0; i < num_paged; i++)
	{
		if (sdram[i].pageno == page)
		{
			sdram[i].access++;
			for (j = 0; j < num_way; j++)
			{
				targetaddress = baseaddress + j;
				if (sssd[targetaddress].valid == 1)
				{
					if (sssd[targetaddress].tag == tag)
					{
						num_hitd++;
						return true;
					}
				}
			}
		}
	}
	num_missd++;
	return false;
}

bool cache::ssdread(unsigned int address)
{
	int i, j,flag=0;
	unsigned int page;
	unsigned int tag;
	unsigned int baseaddress;
	unsigned int targetaddress;

	page = getpages(address);
	tag = gettags(address);
	if (!getbaseaddresss(page, baseaddress))
		return false;

	num_accesss++;
	num_reads++;

	for (j = 0; j < num_way; j++)
	{
		targetaddress = baseaddress + j;
		if (sssd[targetaddress].valid == 1)
		{
			if (sssd[targetaddress].tag == tag)
			{
				num_hits++;
				flag = 1;
				return true;
			}
		}
	}

	if (flag)
	{
		for (i = 0; i < num_paged; i++)
		{
			if (sdram[i].access == 0)
			{
				sdram[i].pageno = page;
				sdram[i].access++;

				sssd[baseaddress].valid = 1;
				sssd[baseaddress].tag = tag;
				return true;
			}
		}
	}

	num_misss++;
	return false;
}

bool cache::cachewrite(unsigned int address)
{
	num_access++;
	return loadandwriteblock(address);
}

bool cache::instread(unsigned int address)
{
	return cacheread(address);
}


bool cache::loadandwriteblock(unsigned int address)
{
	unsigned int way;
	unsigned int baseaddress;
	unsigned int targetaddress;

	num_write++;

	if (!getbaseaddress(getindex(address), baseaddress))
		return false;

	for (way = 0; way < num_associativity; way++)
	{
		targetaddress = baseaddress + way;
		if (scachememory[targetaddress].valid == 0)
		{
			scachememory[targetaddress].valid = 1;
			scachememory[targetaddress].tag = gettag(address);
			return true;
		}
	}
	//when full, execute LRU
	scachememory[baseaddress].tag = gettag(address);
	return loadandwriteblock2(address);;
	return false;
}

bool cache::loadandwriteblock2(unsigned int address)
{
	unsigned int way;
	unsigned int baseaddress;
	unsigned int targetaddress;

	num_write2++;

	if (!getbaseaddress2(getindex2(address), baseaddress))
		return false;

	for (way = 0; way < num_associativity2; way++)
	{
		targetaddress = baseaddress + way;
		if (scachememory2[targetaddress].valid == 0)
		{
			scachememory2[targetaddress].valid = 1;
			scachememory2[targetaddress].tag = gettag2(address);

			return true;
		}
	}
	//when full, execute LRU
	way = rand() % num_associativity2;
	targetaddress = baseaddress + way;
	scachememory2[targetaddress].tag = gettag2(address);
	return loadandwriteblock3(address);

	return false;
}

bool cache::loadandwriteblock3(unsigned int address)
{
	int i, j, flag = 0;
	unsigned int page;
	unsigned int tag;
	unsigned int baseaddress;
	unsigned int targetaddress;

	page = getpages(address);
	tag = gettags(address);
	if (!getbaseaddresss(page, baseaddress))
		return false;

	num_writed++;

	for (i = 0; i < num_paged; i++)
	{
		if (sdram[i].pageno == page)
		{
			sdram[i].access++;
			flag = 1;
			for (j = 0; j < num_way; j++)
			{
				targetaddress = baseaddress + j;
				if (sssd[targetaddress].valid == 0)
				{
					sssd[targetaddress].valid = 1;
					sssd[targetaddress].tag = tag;
					if (num_sbuffer)
						streambufferwrite(address);

					if (num_vbuffer)
						victimbuferwrite(address);
					return true;
				}
			}
		}
	}
	if (flag)
	{
		j = rand() % num_way;
		targetaddress = baseaddress + j;
		sssd[targetaddress].tag = tag;
		if (num_sbuffer)
			streambufferwrite(address);

		if (num_vbuffer)
			victimbuferwrite(address);
		return true;
		//when full, execute LRU

	}
	for (i = 0; i < num_paged; i++)
	{
		if (sdram[i].access == 0)
		{
			sdram[i].pageno = page;
			sdram[i].access++;

			sssd[baseaddress].valid = 1;
			sssd[baseaddress].tag = tag;
			if (num_sbuffer)
				streambufferwrite(address);

			if (num_vbuffer)
				victimbuferwrite(address);

			return true;
		}
	}
	return loadandwriteblock4(address);
}
bool cache::loadandwriteblock4(unsigned int address)
{
	int i, j;
	unsigned int page;
	unsigned int tag;
	unsigned int minaccess,minpage;
	unsigned int baseaddress;
	unsigned int targetaddress;

	page = getpages(address);
	tag = gettags(address);
	if (!getbaseaddresss(page, baseaddress))
		return false;

	num_writes++;
	minaccess = sdram[0].access;
	minpage = 0;
	for (i = 1; i < num_paged; i++)
	{
		if (minaccess < sdram[i].access)
		{
			minaccess = sdram[i].access;
			minpage = i;
		}
	}
	sdram[i].pageno = page;
	for (i = 1; i < num_paged; i++)
	{
		sdram[i].access = 1;
	}

	for (i = 0; i < num_paged; i++)
	{
		for (j = 0; j < num_way; j++)
		{
			targetaddress = baseaddress + j;
			if (sssd[targetaddress].valid == 0)
			{
				sssd[targetaddress].valid = 1;
				sssd[targetaddress].tag = tag;
				if (num_sbuffer)
					streambufferwrite(address);

				if (num_vbuffer)
					victimbuferwrite(address);
				return true;
			}
		}
	}
	
	//when full, execute LRU
	j = rand() % num_way;
	targetaddress = baseaddress + j;
	sssd[targetaddress].tag = tag;
	return true;

	return false;
}
unsigned int cache::gettag(unsigned int address)
{
	return address >> (req_index + req_offset);
}
unsigned int cache::gettag2(unsigned int address)
{
	return address >> (req_index2 + req_offset2);
}
unsigned int cache::gettags(unsigned int address)
{
	return address >> (req_pages + req_offsets);
}
unsigned int cache::getindex(unsigned int address)
{
	return (address & reqindexmask) >> req_offset;
}
unsigned int cache::getindex2(unsigned int address)
{
	return (address & reqindexmask2) >> req_offset2;
}
unsigned int cache::getpages(unsigned int address)
{
	return (address & reqpagemasks) >> req_offsets;
}
bool cache::getbaseaddress(unsigned int index, unsigned int &address)
{
	address = num_associativity*index;
	if (address >= num_block){
		perror("index is invalid");
		return false;
	}
	return true;
}

bool cache::getbaseaddress2(unsigned int index, unsigned int &address)
{
	address = num_associativity2*index;
	if (address >= num_block2){
		perror("index2 is invalid");
		return false;
	}
	return true;
}

bool cache::getbaseaddresss(unsigned int page, unsigned int &address)
{
	address = num_way*page;
	if (address >= num_blocks){
		perror("page is invalid");
		return false;
	}
	return true;
}

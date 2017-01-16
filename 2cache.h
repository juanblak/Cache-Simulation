struct cachememory
{
	unsigned int valid;
	unsigned int tag;

};

struct cachememory2
{
	unsigned int valid;
	unsigned int tag;

};

struct ssd
{
	unsigned int valid;
	unsigned int tag;
	unsigned int pageno;

};

struct dram
{
	unsigned int pageno;
	unsigned int access;

};
struct streambuffer
{
	unsigned int valid;
	unsigned int pageno;
};
struct victimbuffer
{
	unsigned int valid;
	unsigned int pageno;
};

class cache
{
private:
	struct cachememory *scachememory;
	struct cachememory2 *scachememory2;
	struct ssd *sssd;
	struct dram *sdram;
	struct victimbuffer *svictimbuffer;
	struct streambuffer *sstreambuffer;


	unsigned int size_ssd;
	unsigned int size_page;
	unsigned int num_way;
	unsigned int num_pages;
	unsigned int num_blocks;

	unsigned int size_dram;
	unsigned int num_paged;


	unsigned int size_cache;
	unsigned int size_cache2;

	unsigned int size_block;//L:block size
	unsigned int num_associativity;//K
	unsigned int num_set;//N

	//unsigned int size_block2;//L:block size
	unsigned int num_associativity2;//K
	unsigned int num_set2;//N

	unsigned int num_block;
	unsigned int num_block2;

	unsigned int req_offset;
	unsigned int req_index;
	unsigned int req_tag;//bit

	unsigned int req_offset2;
	unsigned int req_index2;
	unsigned int req_tag2;//bit

	unsigned int reqindexmask;
	unsigned int reqoffsetmask;

	unsigned int reqindexmask2;
	unsigned int reqoffsetmask2;

	unsigned int req_offsets;
	unsigned int req_pages;
	unsigned int req_tags;
	unsigned int reqpagemasks;
	unsigned int reqoffsetmasks;

	unsigned int num_vbuffer;
	unsigned int num_sbuffer;
	unsigned int num_vwrite;
	unsigned int num_vread;
	unsigned int num_vhit;
	unsigned int num_swrite;
	unsigned int num_sread;
	unsigned int num_shit;

	unsigned int num_access;
	unsigned int num_miss;
	unsigned int num_hit;
	unsigned int num_read;
	unsigned int num_write;

	unsigned int num_access2;
	unsigned int num_miss2;
	unsigned int num_hit2;
	unsigned int num_read2;
	unsigned int num_write2;

	unsigned int num_accessd;
	unsigned int num_missd;
	unsigned int num_hitd;
	unsigned int num_readd;
	unsigned int num_writed;

	unsigned int num_accesss;
	unsigned int num_misss;
	unsigned int num_hits;
	unsigned int num_reads;
	unsigned int num_writes;

	inline unsigned int gettag(unsigned int address);
	inline unsigned int gettag2(unsigned int address);
	inline unsigned int getindex(unsigned int address);
	inline unsigned int getindex2(unsigned int address);

	inline unsigned int gettags(unsigned int address);
	inline unsigned int getpages(unsigned int address);

	inline bool getbaseaddress(unsigned int index, unsigned int &address);
	inline bool getbaseaddress2(unsigned int index, unsigned int &address);
	inline bool getbaseaddresss(unsigned int page, unsigned int &address);



	bool loadandwriteblock(unsigned int address);
	bool loadandwriteblock2(unsigned int address);
	bool loadandwriteblock3(unsigned int address);
	bool loadandwriteblock4(unsigned int address);



	bool streambufferread(unsigned int address);
	bool streambufferwrite(unsigned int address);

	bool victimbuferread(unsigned int address);
	bool victimbuferwrite(unsigned int address);

public:
	cache(int pagesize ,int vbuffernum,int sbuffernum);
	~cache();
	void deletecache();
	bool initcache();

	bool cacheread(unsigned int address);
	bool cacheread2(unsigned int address);

	bool cachewrite(unsigned int address);
	bool instread(unsigned int address);

	bool dramread(unsigned int address);
	bool ssdread(unsigned int address);


	//unsigned int getblocksize(){ return size_block; };
	//unsigned int getassociativity(){ return num_associativity; };
	//unsigned int getsetnum(){ return num_set; };
	//unsigned int getcachesize(){ return size_cache; };

	//unsigned int getblocksize2(){ return size_block; };
	//unsigned int getassociativity2(){ return num_associativity2; };
	//unsigned int getsetnum2(){ return num_set2; };
	//unsigned int getcachesize2(){ return size_cache2; };

	unsigned int getaccessnum(){ return num_access; };
	unsigned int getmissnum(){ return num_miss; };
	unsigned int gethitnum() { return num_hit; };
	unsigned int getreadnum(){ return num_read; };
	unsigned int getwritenum(){ return num_write; };

	unsigned int getaccessnum2(){ return num_access2; };
	unsigned int getmissnum2(){ return num_miss2; };
	unsigned int gethitnum2() { return num_hit2; };
	unsigned int getreadnum2(){ return num_read2; };
	unsigned int getwritenum2(){ return num_write2; };

	unsigned int getaccessnumd(){ return num_accessd; };
	unsigned int getmissnumd(){ return num_missd; };
	unsigned int gethitnumd() { return num_hitd; };
	unsigned int getreadnumd(){ return num_readd; };
	unsigned int getwritenumd(){ return num_writed; };

	unsigned int getaccessnums(){ return num_accesss; };
	unsigned int getmissnums(){ return num_misss; };
	unsigned int gethitnums() { return num_hits; };
	unsigned int getreadnums(){ return num_reads; };
	unsigned int getwritenums(){ return num_writes; };

	double getmissrate(){ return (num_miss*1.0) / (double)(num_access*1.0); };
	double getmissrate2(){ return (num_miss2*1.0) / (double)(num_access2*1.0); };
	double getmissrated(){ return (num_missd*1.0) / (double)(num_accessd*1.0); };
	double getmissrates(){ return (num_misss*1.0) / (double)(num_accesss*1.0); };

	double totalaccesstime(){
		return (double)(num_write + num_read + num_write2 + num_read2 + num_writed * 10 + num_readd * 10) / 1000000.0
			+ (double)(num_writes * 200 + num_reads * 25) / 1000.0
			+ (double)(num_miss * 1 + num_miss2 * 20 + num_missd * 200) / 1000000.0;
	};

	double amat(){
		return 1 + getmissrate()*(1 + getmissrate2() * (20+getmissrated()*200)) ;
	};

	unsigned int getvbuffernum(){ return num_vbuffer; };
	unsigned int getsbuffernum(){ return num_sbuffer; };

	unsigned int getvwritenum(){ return num_vwrite; };
	unsigned int getvreadnum(){ return num_vread; };
	unsigned int getvhitmnum(){ return num_vhit; };

	unsigned int getswritenum(){ return num_swrite; };
	unsigned int getsreadnum(){ return num_sread; };
	unsigned int getshitmnum(){ return num_shit; };
};
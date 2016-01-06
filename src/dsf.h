//disjoint-set forest data structure with rank and path-compression
//Albert Szmigielski 2012


struct node
{
  int data;
  int rank;
  node *parent;
};



class dsf {




public:
	dsf(); //constructor
	~dsf();
	void MakeSet(node * x, int data_in);
	void MakeSet(int data_in);
	void Union(node * x, node * y);
	void Union(dsf y);
	node * FindSet(node* x);
	node * FindSet();
	node * FindSetR(node* x); 	//read-only version no path compression.
	node * FindSetR(); 			//read-only version no path compression.	
	node * get_node_ptr();
	void print();
	
private:
 	void Link(node* x, node* y);
	node dsf_node;
	
};


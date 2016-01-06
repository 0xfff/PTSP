
#include <iostream>
#include "dsf.h"

using namespace std;

dsf::dsf()
{
	dsf_node.parent = 0;
	dsf_node.rank =0;
	dsf_node.data =0;

}

dsf::~dsf(){}


void dsf::Link(node* x, node* y)
{
	if (x->rank > y->rank)
		y->parent = x;
	else {
		x->parent = y;
		if (x->rank == y->rank)
			y->rank ++ ;
	 }
}


void dsf::MakeSet(node * x, int data_in) 
{
	x->parent = x;
	x->rank = 0;
	x->data = data_in;
}
void dsf::MakeSet(int data_in){
	node * x = get_node_ptr();
	x->parent = x;
	x->rank = 0;
	x->data = data_in;

}
void dsf::Union(node * x, node * y){
	Link(FindSet(x), FindSet(y));
}

void dsf::Union(dsf y){
	Link(FindSet(), y.FindSet());
}



node *dsf::FindSet(node* x)
{
	if (x!=x->parent)
		x->parent = FindSet(x->parent);
	return x->parent;
}

node *dsf::FindSet()
{
	node * x = get_node_ptr();
	if (x!=x->parent)
		x->parent = FindSet(x->parent);
	return x->parent;
	//if (this!=this.node->parent)
	//	this.node->parent = FindSet(this.node->parent);
	//return get_node_ptr()->parent;
}

node *dsf::FindSetR(node* x) //read-only version, no path compression.
{
	if (x!=x->parent)
		return FindSetR(x->parent);
	return x->parent;
}

node *dsf::FindSetR() 		//read-only version, no path compression.
{
	node * x = get_node_ptr();
	if (x!=x->parent)
		return FindSetR(x->parent);
	return x->parent;
}

node * dsf::get_node_ptr()
{
	return &dsf_node;
}

void dsf::print() {
	cout << " dsf_node->parent " << dsf_node.parent << "\t";
	cout << " dsf_node->rank " << dsf_node.rank << "\t";
	cout << " dsf_node -> data " <<dsf_node.data << "\t";
	cout << endl;
}

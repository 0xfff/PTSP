//Albert Szmigielski 2012

// g++ -g  dsf.cpp par_omp_sort.cpp -o par_omp_sort -fopenmp 
// 
// usage ./tsp --file <inputfile> --start <start city number> --threads <number of threads>
// --sort <1=parallel sort 0=std::sort>
// e.g. to run on file usa.in with 4 threads and parallel sort enter:
//    ./tsp -file usa.in -sort 1 -threads 4

#include <time.h>
#include <algorithm>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <list>
#include <omp.h>
#include <ctime>
#include <thread>
#include <getopt.h>

#include "dsf.h"


using namespace std;


struct edge {
	int x;
	int y;
	float weight;
};

static float ** Cities, ** Distance, **Edges;
static string * CityNames;
int cities, num_edges, TourCity=0, Next, CycleEdge = 1, MsfEdge = 2, sort_type=0;
ofstream outfile;
float Min_x=1000, Min_y=1000, Max_x=0, Max_y=0;
list<int>::iterator it;
int * edge_color_helper, *edge_color_main;
int Num_Threads=1;
double start_s=0;
/*******************   experiment With Starting City    *********************/
/*
void experimentWithStartingCity(){
	float min_tour =tour_length;


	for (int i=0; i<cities;  i++)
	{	
		createAdjList(TreeEdgeList, AdjList);
		TourCity=0;	
		pre_order(AdjList, tour, i);
		tour[cities] =i; // come back home
		tour_length = calcTourLength(tour);
		cout << tour_length << endl;
		if (tour_length < min_tour) {
			min_tour = tour_length;
		}	
	}
	cout << "MIn tour Distance = " << min_tour << endl;
}
*/
/********************    Calc Tour Length    *************************/
double calcTourLength(int * Tour){
	double tour_l=0;
	for (int i=0; i<cities; i++){
		tour_l += Distance[Tour[i]][Tour[i+1]];
	}
	return tour_l;
}
/********************    PRE-ORDER     *************************/
void pre_order(list<int> * AdjList, int * Tour, int current){
	/*pre-order walk of the tree starting at vertex current
	  this will yield an TSP tour approximation with bound =2
	*/
	
	Tour[TourCity] = current;
	TourCity++;
	while (!AdjList[current].empty()) 
		{
		Next = AdjList[current].front();
		AdjList[current].remove(Next);
		AdjList[Next].remove(current);		
		pre_order(AdjList, Tour, Next);
		}
	

}

/********************    Create Adj List     *************************/
void createAdjList(edge * TreeEdgeList, list<int> * AdjList){
	int x,y;
	for (int i=0; i<cities-1; i++) {
		x = TreeEdgeList[i].x;
		y = TreeEdgeList[i].y;
		AdjList[x].insert(AdjList[x].begin(),y);
		AdjList[y].insert(AdjList[y].begin(),x);
	}

}
/**********************   PrintList()     *********************************/
void PrintList(list<int>  my_list){
cout << " list  contains:";
  for (it=my_list.begin(); it!=my_list.end(); it++)
    cout << " " << *it;
  cout << endl;

}

/********************    Print Input      *************************/
void PrintInput() {
for (int i=0; i<cities;  i++)
	{
		cout << Cities[i][0] << "\t " << Cities[i][1] << "\t " << CityNames[i] << endl; // 
	}
}
/********************    Print Edges     *************************/
void PrintEdges(int n)
{
	for (int i=0; i<n; i++) {
		cout << Edges[i][0] <<"\t" << Edges[i][1] <<"\t" << Edges[i][2] <<"\n";
	}
}
/********************    Sort Edges Ops     *************************/
bool myComp(float  *i, float *j){
	//cout << i[0] << " " << j[0] << " i " << i << "\n";
	return (i[0] < j[0]) ;
}

struct myclass{
	bool operator() (edge  i, edge j) 
	{	
		//if (i.x == j.x) return (i.y < j.y);
		//else 
		return (i.x < j.x);
	}
} myedge;

/********************    Test the Sort    *************************/
void TestSort()
{
	cout << "SORT \n";
	
	PrintEdges(num_edges);
	cout << "Edges[0][0] = " << Edges[0][0] << "\n";
	cout << "&Edges[0] = " << &Edges[0] << "\n";
	if (myComp(Edges[0], Edges[1])) 
		cout << " Edges[0] " << Edges[0][0] << " <  Edges[1] " << Edges[1][0] << " \n";
	else cout << " Edges[1] " << Edges[1][0] << " <  Edges[0] " << Edges[0][0] <<" \n";
			
}

/********************    Calculate Distance matrix      *************************/
void CalcDistance() {
// Calculate Distance Matrix
	
	
	
		for (int i=0; i<cities;  i++)
		{
			#pragma omp parallel for num_threads(Num_Threads)
			for (int j=0; j<cities;  j++){
				if (i==j) Distance[i][j]=0.0;
				if (i<j) 
				{	Distance[i][j] = sqrt( (pow(Cities[i][0] - Cities[j][0],2))  + (pow(Cities[i][1] - Cities[j][1],2))) ;
				}
				else Distance[i][j]= Distance[j][i];
			
			}
		
		}

	int counter=0;
	for (int i=0; i<cities;  i++)
		{
			for (int j=0; j<cities;  j++){
				if (i<j){
				//__transaction_atomic { 			
						Edges[counter][0]=Distance[i][j];
						Edges[counter][1]=i;
						Edges[counter][2]=j;
						counter++;
					//}
				}
			}
		}
			
}
/********************    Test Distance matrix      *************************/
void PrintDistance(){
	for (int i=0; i<cities;  i++)
		{
			for (int j=0; j<cities;  j++){
				cout << Distance[i][j]<< "\t"; // 
			}
		cout << endl;
	}
}
/********************    Write Tour To File     *************************/
void WriteTourToFile(char* name, int* tour){
	char  fname[]=" ";
	strcpy(fname,name);

	
	strcat (fname, "_tour");
	cout << fname << endl;
	//cout << filename << endl;
	outfile.open(fname);
	outfile << name << endl;
	outfile << int(100*Min_x) << " " << int(100*Max_x) << " ";
	outfile << int(100*Min_y) << " " << int(100*Max_y) << " " ;
	outfile << endl;
	int A_x, A_y, B_x, B_y;
	
	for (int i=0; i<cities;  i++)
		{
			//cout << "City number: " << 	tour[i] << " ";		
			//outfile << Distance[tour[i]][tour[i+1] << endl;
			A_x = int(100 * Cities[tour[i]][0]);
			A_y = int(100 * Cities[tour[i]][1]);
			B_x = int(100 * Cities[tour[i+1]][0]);
			B_y = int(100 * Cities[tour[i+1]][1]);	
			outfile << "+ ";
			outfile << B_x << " ";
			outfile << B_y << " ";
			outfile << A_x << " ";
			outfile << A_y << " ";
			outfile << endl;
		}

	outfile.close();
}

/**************************    PAR MERGE    ******************************/
void par_merge(int num_lists, int mul, int interval){
		//merge until 1 list left
		//merge list 1&2, 3&4, ... (n-1 & n)
		//cout << "\n *** Merge Begin ***\n";
	mul = mul*2;
	int par_merge_threads = num_lists/2;
	if (num_lists > 1){
		#pragma omp parallel num_threads(par_merge_threads)
		{
			int start = interval * omp_get_thread_num() *mul;
			int end = interval * (omp_get_thread_num()+1) *mul ;
			int middle = floor(start+ (end - start)/2);
			if (omp_get_thread_num() == par_merge_threads-1) {end=num_edges;}
			/*
			#pragma omp critical 
			{
				cout << " Thread # " << omp_get_thread_num();
				cout << " start: " << start;
				cout << " middle: " << middle;
				cout << " end: " << end << endl;
			}	
			*/
			inplace_merge( &Edges[start],&Edges[middle], &Edges[end], myComp);			
		}
		num_lists = ceil(num_lists/2);
			//cout << "num_lists: "  << num_lists << endl;
		//cout << " --- \n";
		par_merge (num_lists, mul, interval);		
	}		
}	

/**************************    PAR SORT    ******************************/
void par_sort () {
	// break into num_threads chunks, sort each chunk
	int interval = floor(num_edges/Num_Threads);
	#pragma omp parallel num_threads(Num_Threads)
	{	

		int start = interval * omp_get_thread_num();
		int end = interval * (omp_get_thread_num()+1) ;
		if (omp_get_thread_num() == Num_Threads-1) end=num_edges;
		sort (&Edges[start], &Edges[end], myComp);
		/*
		#pragma omp critical 
		{
			cout << " Thread # " << omp_get_thread_num();
			cout << " start: " << start;
			cout << " end: " << end << endl;
		}
		*/
	}
		//cout << " sort done \n";
		//cout << "omp t num: " << omp_get_num_threads() << endl;
	par_merge(Num_Threads, 1, interval);
}


/*****************************    KRUSKAL     *********************************/
void MST_Kruskal_Main(float ** DistArray, edge * EdgeList, dsf * dsf_array)
// Kruskal's MST alg. returns an adjacency list (array of linked lists) of the MST 
// input: DistArray - a matrix of distances between vertices
//		  EdgeList - pointer to the array where the MST will be stored 
{
	int count = 0, edges_looked_at=0;

	//PrintEdges(num_edges);
	for (int i =0; i<num_edges; i++){
		if (edge_color_helper[i] != CycleEdge){
			edges_looked_at++;
			if ((dsf_array[(int)Edges[i][1]].FindSet() ) != (dsf_array[(int)Edges[i][2]]).FindSet() ) {
				//add edge to tree
				//cout << "Looking at edge " << (int)Edges[i][1] << " - " << (int)Edges[i][2]<<endl;
				EdgeList[count].x = (int)Edges[i][1];
				EdgeList[count].y = (int)Edges[i][2];
				EdgeList[count].weight = Edges[i][0];
				count++;	
				//Union (u,v)
				dsf_array[(int)Edges[i][1]].Union(dsf_array[(int)Edges[i][2]]);
				edge_color_main[i] = MsfEdge;
			}
			else edge_color_main[i] = CycleEdge;
		}
	}
	cout << " edges: " << edges_looked_at << "\t";
}

/**************************    KRUSKAL HELPER    ******************************/
void MST_Kruskal_Helper(dsf * dsf_array, int part_start, int part_end) {

	//assign a partition of Edges to each helper thread
	//initilize partition
	int count=0 ;
	 //printf("part start: %d \n" , part_start); 
	if (edge_color_main[part_start] == 0){
		for (int i=part_start; i<=part_end; i++){
			if (edge_color_helper[i] == 0){
					if ((dsf_array[(int)Edges[i][1]].FindSetR() ) == (dsf_array[(int)Edges[i][2]]).FindSetR() )
					{		
						edge_color_helper[i] = CycleEdge;
						count++;
						//cout << ".";
					}
				//}
			}		
		}
	}
	//cout << count << endl;
}
/*****************************    KRUSKAL  PAR   *********************************/
void MST_Kruskal_Par(float ** DistArray, edge * EdgeList){
	dsf dsf_array[cities];
	//#pragma omp parallel for 
		for (int i=0; i<cities;  i++)
		{
			dsf_array[i].MakeSet( dsf_array[i].get_node_ptr(), i);	
		}
	#pragma omp parallel for 
		for (int i=0; i<num_edges;  i++)
		{
			edge_color_main[i] = 0;
			edge_color_helper[i] = 0;		
		}
	//Sort all the edges
	//cout << "s_type: " << sort_type;
	if (sort_type) par_sort ();
	else sort (&Edges[0], &Edges[num_edges], myComp);
	//TestSort();
	
	for (int i=0; i<num_edges;i++){	
		if (i>0){
			if (Edges[i][0] < Edges[i-1][0]) cout << "Error ";
		}
		else 
			if (Edges[i][0] > Edges[i+1][0]) cout << "Error ";
	}
	
	
	#pragma omp parallel num_threads(Num_Threads)
	{		
		int start =0, end=0; //start & end for helper thread partitions
						// calculate these.
		if (omp_get_thread_num() == 0){
			MST_Kruskal_Main(Distance, EdgeList, dsf_array);
			//cout << "Main done" <<endl;
		}
		else{
			start = num_edges/Num_Threads * omp_get_thread_num();
			end = start + num_edges/Num_Threads -1;
			if (omp_get_thread_num() == Num_Threads-1) {end = num_edges-1;}
			//printf("thread: %d has start %d and end %d\n", omp_get_thread_num(), start, end);			
			MST_Kruskal_Helper(dsf_array,start,end);
			//cout << "Helper " << omp_get_thread_num() <<" done" <<endl;
		}
	}
}



/**************************    check MST    ******************************/
float checkMST(edge * TreeEdgeList){
	sort(&TreeEdgeList[0] , &TreeEdgeList[cities-1], myedge);
	//sort by the x coordinate
	float sum=0.0;
	#pragma omp parallel for num_threads(Num_Threads)
	for (int i =0; i<cities-1; i++){
		//cout << TreeEdgeList[i].x << "\t";
		//cout << TreeEdgeList[i].y << "\t" << TreeEdgeList[i].weight;
		//cout << endl;
		__transaction_atomic { 
			sum = sum + TreeEdgeList[i].weight;
		}
	}
	//cout << " MST Sum = " << sum << endl;
	return sum;
	
}

/*****************************    WriteTreeToFile   ***************************/
void WriteTreeToFile(edge * EdgeList, char * name){
	
	//write to file
	char  fname[]=" ";
	strcpy(fname,name);	
	strcat (fname, "_mst");
	outfile.open(fname);
	outfile << fname << endl;

	outfile << int(100*Min_x) << " " << int(100*Max_x) << " ";
	outfile << int(100*Min_y) << " " << int(100*Max_y) << " " ;
	outfile << endl;
	int A_x, A_y, B_x, B_y;
	
	for (int i =0; i<cities-1; i++){
		A_x = int(100 * Cities[EdgeList[i].x][0]);
		A_y = int(100 * Cities[EdgeList[i].x][1]);
		B_x = int(100 * Cities[EdgeList[i].y][0]);
		B_y = int(100 * Cities[EdgeList[i].y][1]);	
		outfile << "+ ";
		outfile << B_x << " ";
		outfile << B_y << " ";
		outfile << A_x << " ";
		outfile << A_y << " ";
		outfile << endl;
		
	}
	outfile.close();
	// end write to file
}
/*****************************    DISPLAY TOUR     *********************************/
void DisplayTour(int* tour){
	cout << "Here is your Tour: \n";
	for (int i=0; i<cities;  i++)
		{
			//cout << "City number: " << 	tour[i] << " ";	
			cout << CityNames[tour[i]];
			if (i<cities)
			 	cout << "\t" << Distance[tour[i]][tour[i+1]] ;
			cout << endl;
		}
}
/*****************************    GET SECONDS     *********************************/
double get_seconds()
  {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    double tv_sec = double(ts.tv_sec);
    double tv_nsec = double(ts.tv_nsec);
    double seconds = tv_sec + (tv_nsec / double(1000000000.0));
    return seconds;
  }
/*****************************    ARGS     *********************************/
static struct option long_options[] =
  {
    {"file", required_argument, 0, 'f'},
    {"start", required_argument, 0, 's'},
    {"threads", required_argument, 0, 't'},
	{"sort", required_argument, 0, 'o'},
    {0, 0, 0, 0}
  };

/*****************************    MAIN     *********************************/
int main(int argc, char** argv)
{
	//printf("working... \t");
	int i,j=0, startCity=0 ;
	float x,y, mst_sum;
	double tour_length=0;
	string City;
	char f[]="data/idaho.in";
	char  * filename;
	filename=f;
	
// Code based on the example code in the getopt documentation
  while (true) {

    int option_index = 0;    
    int c = getopt_long_only(argc, argv, "f:s:t:o",
                             long_options, &option_index);
    
    /* Detect the end of the options. */
    if (c == -1)
      break;
    
    switch (c) {
    case 0:
      /* If this option set a flag, do nothing else now. */
      break;
      
    case 'f':
      filename = optarg;
      break;
      
    case 's':
      startCity = atoi(optarg);
      break;
      
    case 't':
      Num_Threads = atoi(optarg);
      break;
	   
	case 'o':
      sort_type = atoi(optarg);
      break;
         
    case '?':
      /* getopt_long already printed an error message. */
      exit(1);
      break;
      
    default:
      exit(1);
    }
  }
	cout << "t's:" << Num_Threads << "\t";
	//cout << filename;
	//if (argc >1) filename = argv[1];
	//if (argc>2) startCity = atoi(argv[2]); 
  	ifstream myfile (filename);
  	if (myfile.is_open())
  	{
		if (myfile.good())  myfile>>cities; //cities is number of cities
		cout << "v's: " << cities << " ";
		Cities = new float *[cities]; 		// 2-dim array to hold x,y coordinates
		Distance = new float *[cities]; 	// cities by cities array to hold dist. between cities
		num_edges = (cities) * (cities-1) / 2 ; // num of edges in a comp G = n*(n-1)/2
		edge_color_helper = new int[num_edges];
		edge_color_main = new int[num_edges];
		Edges = new float *[num_edges];			// array to hold all edges
		for (int i=0; i< cities; i++) {
			Cities[i] = new float[2];
			Distance[i] = new float[cities];
			}
		for (int i=0; i<num_edges;i++){		// array Edges holds arrays of
			Edges[i] = new float[3];		// 0.edge weight 1. from_city 2.to_city 
			}
		CityNames = new string[cities];
    	while (myfile >>x>>y ) //myfile.good()
    	{		
			getline (myfile, City);
			Cities[j][0] = x;
 			Cities[j][1] = y;
			CityNames[j] = City;
			if (x > Max_x) {Max_x = x;}
			if (x < Min_x) {Min_x = x;}
			if (y > Max_y) {Max_y = y;}
			if (y < Min_y) {Min_y = y;}
			j++;
      		//cout << x << "\t " << y << "\t "  << City << endl; //
    	}
    	myfile.close();
		
	}
  	else { cout << "Unable to open file \n"; return 0;}

	edge TreeEdgeList[cities];
	list<int> AdjList[cities];
	int tour[cities+1];

		//PrintInput(); 
		//PrintDistance();
	CalcDistance();
	//cout<< " starting timer...";
	
	start_s=omp_get_wtime();
		//cout<<" Done calculating distance...";
		MST_Kruskal_Par(Distance, TreeEdgeList);
		//cout << "done MST" <<endl;	
	int stop_s=omp_get_wtime();
	//cout<< " stopping timer" << endl;
	cout << "time: " << (stop_s-start_s) << "\t";
		//PrintEdges(num_edges);
		mst_sum=checkMST(TreeEdgeList);
		createAdjList(TreeEdgeList, AdjList);
		if ((startCity > cities) || (startCity <0 )) startCity=0;
		pre_order(AdjList, tour, startCity);
		tour[cities] =startCity; // come back home
	//end = gethrtime();
	
	//WriteTreeToFile(TreeEdgeList, filename);
	//DisplayTour(tour);
	//WriteTourToFile(filename, tour);
	tour_length = calcTourLength(tour);
	cout << "MST= " << mst_sum << "\t";
	cout << "TSP= " << tour_length << "\n";
	
	//cout << "bye bye" << endl;
 
return 0;
}



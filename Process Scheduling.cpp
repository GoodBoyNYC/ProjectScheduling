/*
Llubomir Stanojlovic
Process Scheduler
*/
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <list>
#include <queue>
using namespace std;

struct Process{
	Process():ID("IDLE"),burst(0),lastBurst(0),preempt(false){}
	string ID;
	int arrivalTime;
	int TotalCpu;
	int totalBurstTime;
	int burst;
	int avgBurst;
	int IOwait;
	double burstPredict;
	double lastBurst;
	bool preempt;
	bool operator!(){
		if(ID == "IDLE")
			return true;
		return false;
	}
	bool compareProcess(Process any){
		return (this->burstPredict > any.burstPredict);
	}
};

struct Scheduling{
	string ProcFile;		 //The ID of the file with the Processes to be scheduled.
	int ioDelay;		
	int contextSwitchDelay;
	int CTSSqueues;

	void scheduler(){   //Displays what processes are going to be scheduled from a specific file.
		cout << "Process File = " << ProcFile << endl
			<< "IoDelay = " << ioDelay << endl
			<< "Context Switch Delay = " << contextSwitchDelay << endl
			<< "CTSS queues = " << CTSSqueues << endl;
	}
};

void openFile(ifstream& fileInput, string& fileName);
void readResource(ifstream& fileInput, Scheduling& task);
void getProcesses(ifstream& file, list<Process>& proc);
void checkBlocks(queue<Process>& ready, vector<Process>& blocked, int time);
void blockProcess(Process& active, vector<Process>& blocked, Scheduling& sch, bool& contextSwitch, int& delayTime);
bool isIdle(const Process& running, ifstream& rFile, const int time);
void printSpec(list<Process>& proc, Scheduling& sch);

void runFCFSAlg(list<Process> p, Scheduling& s){
	string randomnum("random.txt");
	queue<Process> ready;		//Stores process that are ready to run
	vector<Process> blocked;	//Stores blocked processes
	Process running;			//Stores the running process
	bool cSwitch(false);		//Shows if a context switch has occured
	int delayTime(0);			//Delay time for context switch
	ifstream input;             // File for Random numbers
	openFile(input,randomnum);  //Opens random number file
	cout << "in FCFS";
	int time(0);				  //Indicates the current time
	
	while(true)
	{
		if(ready.empty() && blocked.empty() && p.empty() && !running )
			break;
		if(cSwitch && delayTime == 0)
			cSwitch = false;
		while(!p.empty() && p.front().arrivalTime == time)
		{
			cout << "TIME " << time << ": ";
			ready.push(p.front());
			cout << "Moving process " << p.front().ID << " from arrival to ready."<<endl;
			p.pop_front();
		}

		checkBlocks(ready, blocked,time);

		if(!cSwitch && !running && !ready.empty())
		{
			cout << "TIME " << time << ": ";
			running = ready.front();
			cout << "Moving process " << ready.front().ID << " from ready to running." 
				<< "  Remaining time: " << ready.front().totalBurstTime << endl;
			ready.pop();
		}
		if(running.ID != "NULL" && !cSwitch && isIdle(running, input,time))
			blockProcess(running, blocked, s, cSwitch, delayTime);
		time ++;
		running.burst++;
		running.totalBurstTime--;
		if(cSwitch == true)
		{
			delayTime--;
		}
	}
}
void beginScheduling(Scheduling& task, list<Process>& proc,ifstream& file, string& schFileName);
void fixPrediction(Process& proc, double ratio);

double getProb(ifstream& ranFile);

int main(){
	string schFileName("scheduling.txt");
	Scheduling sch;
	list<Process> proc;
	ifstream file;
	beginScheduling(sch,proc,file,schFileName);
	printSpec(proc,sch);
	cout << "\n===========FCFS================\n\n"; 
	runFCFSAlg(proc,sch);
}



void getProcesses(ifstream& file, list<Process>& proc){
	Process current; 
	string currentID; 
	int number; 
	while(file >> currentID){
		current.ID = currentID;
		file >> number;
		current.arrivalTime = number;
		file >> number;
		current.totalBurstTime = number;
		file >> number;
		current.avgBurst = number;
		proc.push_back(current);
	}
}

void beginScheduling(Scheduling& task, list<Process>& proc,ifstream& file, string& schFileName){
	openFile(file,schFileName);
	readResource(file,task);
	file.clear();
	file.close();
	openFile(file,task.ProcFile);
	getProcesses(file,proc);
	file.clear();
	file.close();
}   

void fixPrediction(Process& proc, double ratio){
	proc.burstPredict = (ratio*(proc.lastBurst))+((1-ratio)*(proc.burstPredict));
}

void checkBlocks(queue<Process>& ready, vector<Process>& blocked, int time){
	if(blocked.empty())
		return;
	for(size_t i = 0; i < blocked.size(); i++)
		blocked[i].IOwait--;
	if(blocked.front().IOwait == 0)
	{
		cout << "TIME " << time << ": ";
		cout << "Moving process " << blocked.front().ID << " from waiting to ready." << endl;
		ready.push(blocked.front());
		blocked.erase(blocked.begin());
	}
}

void blockProcess(Process& active, vector<Process>& blocked, Scheduling& sch, bool& contextSwitch, int& delayTime)
{
	if(active.totalBurstTime > 0)
	{
		active.IOwait = sch.ioDelay;
		active.burst = 0;
		blocked.push_back(active);
	}
	active = Process();  
	contextSwitch = true;
	delayTime = sch.contextSwitchDelay;
}

void openFile(ifstream& fileInput, string& fileName){

	fileInput.open(fileName.c_str());
	if(!fileInput){
		cerr << "Could not open file.";
		exit(1);
	}
}

void readResource(ifstream& fileInput, Scheduling& task){

	string line;
	while(getline(fileInput,line,'=')){
		if(line == "ProcessFile"){
			getline(fileInput,task.ProcFile);
		}
		else if(line == "IOdelay"){
			fileInput >> task.ioDelay;
			fileInput.ignore(1);                          //ignore moves the input stream pointer past the '\n' char.
		}
		else if(line == "ContextSwitchDelay"){
			fileInput >> task.contextSwitchDelay;
			fileInput.ignore(1);
		} 
		else if(line == "CTSSQueues"){
			fileInput >> task.CTSSqueues;
			fileInput.ignore(1);
		}		
		else{
			cerr << "UNKNOWN PARAMETER!!!\n";
		}
	}
} 
bool isIdle(const Process& running, ifstream& rFile, const int time)
{
	if(running.totalBurstTime == 0)
	{
		cout << "TIME " << time << ": " << "Process " << running.ID << " finished." << endl;
		return true;
	}
	else if(running.burst < running.avgBurst-1)
		return false;
	else if(running.burst == running.avgBurst-1)
	{
		if(getProb(rFile) <= (1.0/3.0))
		{
			cout << "TIME " << time << ": " << "Process " << running.ID << " ending burst (" 
				 << running.burst << ") "  << "Remaining time: " << running.totalBurstTime << endl;
			return true;
		}
		else return false;
	}
	else if(running.burst == running.avgBurst)
	{
		if(getProb(rFile) <=  .5) 
		{
			cout << "TIME " << time << ": " << "Process " << running.ID << " ending burst (" 
				<< running.burst << ") "  << "Remaining time: " << running.totalBurstTime << endl;
			return true;
		}
		else return false;
	}
	else
	{ 
		cout << "TIME " << time << ": " << "Process " << running.ID << " ending burst (" 
			<< running.burst << ") "  << "Remaining time: " << running.totalBurstTime << endl;
		return true;
	}
}

double getProb(ifstream& ranFile)
{
	double i;
	string randomNum("random.txt");
	if(ranFile)
	{
		ranFile >> i;
		i/=INT_MAX;
	}
	else
	{
		ranFile.clear();
		ranFile.close();
		openFile(ranFile,randomNum);

		ranFile >> i;
		i/=INT_MAX;
	}
	return i;
}

void printSpec(list<Process>& proc, Scheduling& sch)
{
	cout << "Llubomir Stanojlovic\nProcess Scheduling\nCS3224\nSpring 2012\n";
	cout << "===========\n\n";
	sch.scheduler();
	cout << endl;
	
	list<Process>::const_iterator i;

	for(i = proc.begin();i != proc.end();i++)
	{
		cout << "PID: " << i->ID <<"\t"
			 << "Arrival: " << i->arrivalTime <<"\t"
			 << "Total time: " << i->totalBurstTime<<"\t" 
			 << "Avg Burst: " << i->avgBurst <<"\n";
	}
}
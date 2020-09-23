#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>


// to handle
// 1) write check   done
// 2) read check    done
// 3) main memory   done
// 4) dirty bit     done

using namespace std;

typedef struct cell{
    bool valid;
    int tag;
    int data;
    int priority; // 0 for LOWER PRIORITY and 1 for HIGH PRIORITY
    int last_call;
    int dirty_bit;
} cell;

void print_cache(vector<vector<cell>> cache){
    cout<<"----------------------CACHE----------------------"<<endl;
    cout<<"#Data, Tag, Valid-status(valid=1), dirty-status(dirty=1)"<<endl;
    for(int i = 0; i < cache.size(); ++i){
        cout<<"Set No. "<<(i + 1)<<" -> \n";
        for(int j = 0; j < cache[i].size(); ++j){
            cout<<cache[i][j].data<<" | "<<cache[i][j].tag<<" | "<<cache[i][j].valid<<" | "<<cache[i][j].dirty_bit<<" | "<<cache[i][j].priority<<endl;            
        }
    }
    cout<<endl;
}

void process(vector<vector<cell>> &v, vector<string> input, vector<int> &main_memory, int assoc, int size, int T, int &read_hit, int &read_miss, int &write_hit, int &write_miss){
    if(input[1] == "R"){
        int target = stoi(input[0]);
        int set_number = target%size;
        int current_tag = target/size;
        bool hit = false;
        for(int i = 0; i < assoc; ++i){
            //checking for read hit
            if(v[set_number][i].valid && v[set_number][i].tag == current_tag){
                read_hit++;
                hit = true;
                vector<cell> temp = v[set_number];
                cell temp_cell = temp[i];
                temp.erase(temp.begin() + i);
                vector<cell> temp2 (v[set_number].size());
                temp2[0] = temp_cell;
                for(int j = 1; j < v[set_number].size(); ++j){
                    temp2[j] = temp[j - 1];
                }
                temp2[0].last_call = 0;
                temp2[0].priority = 1;
                for(int j = 1; j < v[set_number].size(); ++j){
                    temp2[j].last_call++;
                    if(temp2[j].last_call >= T && temp2[j].priority == 1)
                        temp2[j].priority = 0;
                }      
                for(int j = 0; j < size; ++j){
                    for(int k = 0; k < assoc; ++k){
                        if(j != set_number){
                            v[j][k].last_call++;
                            if(v[j][k].last_call >= T && v[j][k].priority == 1)
                                v[j][k].priority = 0;
                        }
                    }
                }
                v[set_number] = temp2;
                cout<<"The value in the requested position is "<<temp2[0].data<<endl;
                break;                           
            }
        }
        if(!hit){
            //read miss case handled 
            read_miss++;
            cell add;
            add.data = main_memory[target];
            add.priority = 0;
            add.valid = true;
            add.last_call = 0;
            add.tag = current_tag;
            add.dirty_bit = 0;

            //setting priorities
            for(int j = 0; j < v.size(); ++j){
                for(int k = 0; k < v[j].size(); ++k){
                    v[j][k].last_call++;
                    if(v[j][k].last_call >= T && v[j][k].priority == 1)
                        v[j][k].priority = 0;
                }
            }

            int index = -1;
            int valid_count = 0;

            for(int i = 0; i < v[set_number].size(); ++i){
                if(v[set_number][i].priority == 0 && v[set_number][i].valid && index == -1){
                    index = i;
                }

                if(v[set_number][i].valid)
                    valid_count++;
            }

            if(index != -1){
                //add at given spot
                vector<cell> temp2 = v[set_number];
                
                if(valid_count == v[set_number].size()) {
                    cell remove = temp2[temp2.size() - 1];
                    main_memory[remove.tag*size + set_number] = remove.data;
                    cout<<"Value removed from "<<remove.tag*size + set_number<<endl;
                }   

                temp2[index] = add;

                for(int i = index; i < v[set_number].size() - 1; ++i){
                    temp2[i + 1] = v[set_number][i];
                }

                v[set_number] = temp2;

                return;

            }
            else{
                if(valid_count == v[set_number].size()){
                    cell remove = v[set_number][v[set_number].size() - 1];
                    main_memory[remove.tag*size + set_number] = remove.data;
                    cout<<"Value removed from "<<remove.tag*size + set_number<<endl;

                    v[set_number][v[set_number].size() - 1] = add;
  
                    return;  
                }
                else{
                    int pos = -1;
                    

                    for(int i = 0; i < v[set_number].size(); ++i){
                        if(!v[set_number][i].valid){
                            pos =  i;
                            break;
                        }
                    }


                    // adding first element to low priority 

                    vector<cell> temp2 = v[set_number];
                    temp2[pos] = add;
                    v[set_number] = temp2;
                    
                    return;

                }
            }

            

        }

    }
    else{
        int target = stoi(input[0]);
        int set_number = target%size;
        int current_tag = target/size;
        int data = stoi(input[2]);
        bool hit = false;
        for(int i = 0; i < v[set_number].size(); ++i){
            // in this loop we check for the hit of write instruction
            if(v[set_number][i].valid && v[set_number][i].tag == current_tag){
                write_hit++;
                hit = true;
                v[set_number][i].data = data;
                vector<cell> temp = v[set_number];
                cell temp_cell = v[set_number][i];
                vector<cell> temp2(temp.size());
                temp2[0] = temp_cell;
                temp2[0].last_call = 0;
                temp2[0].priority = 1;
                temp2[0].dirty_bit = 1;
                temp.erase(temp.begin() + i);
                for(int j = 1; j < v[set_number].size(); ++j){
                    temp2[j] = temp[j - 1];
                }
                v[set_number] = temp2;
                for(int j = 0; j < v.size(); ++j){
                    for(int k = 0; k < v[j].size(); ++k){
                        if(j == set_number  && k == 0)
                            continue;
                        else{
                            v[j][k].last_call++;
                            if(v[j][k].last_call >= T && v[j][k].priority == 1)
                                v[j][k].priority = 0;
                        }
                    }
                }
                cout<<"Value written at "<<target<<endl;
                break;
            }
        }
        if(!hit){
            write_miss++;
            //case when it is a miss 
            vector<cell> temp = v[set_number];
            cell add;
            add.priority = 0;
            add.last_call = 0;
            add.data = data;
            add.tag = current_tag;
            add.valid = true;
            add.dirty_bit = 1;

            //setting priorities
            for(int j = 0; j < v.size(); ++j){
                for(int k = 0; k < v[j].size(); ++k){
                    v[j][k].last_call++;
                    if(v[j][k].last_call >= T && v[j][k].priority == 1)
                        v[j][k].priority = 0;
                }
            }

            int index = -1;
            int valid_count = 0;
            
            for(int i = 0; i < v[set_number].size(); ++i){
                if(v[set_number][i].priority == 0 && v[set_number][i].valid && index == -1){
                    index = i;
                }
                if(v[set_number][i].valid)
                    valid_count++;
            }

            if(index != -1){
                vector<cell> temp2 = v[set_number];

                if(valid_count == v[set_number].size()) {
                    cell remove = temp2[temp2.size() - 1];
                    main_memory[remove.tag*size + set_number] = remove.data;
                    cout<<"Value removed from "<<remove.tag*size + set_number<<endl;
                }                

                temp2[index] = add;
                
                for(int i = index; i < v[set_number].size() - 1; ++i){
                    temp2[i + 1] = v[set_number][i];
                }

                v[set_number] = temp2;

                cout<<"Value written at "<<target<<endl;
                return;
            }
            else{
                if(valid_count == v[set_number].size()){
                    // if all elements in cache are high priority
                    vector<cell> temp2 = v[set_number];
                    cell remove = temp2[temp2.size() - 1];
                    main_memory[remove.tag*size + set_number] = remove.data;
                    cout<<"Value removed from "<<remove.tag*size + set_number<<endl;
                    temp2[temp2.size() - 1] = add;                                        
                    v[set_number] = temp2;
                    cout<<"Value written at "<<target<<endl;
                    return;
                }
                else{
                    //if there are no low priority elements but some high priority data
                    int pos = -1;

                    for(int i = 0; i < v[set_number].size(); ++i){
                        if(!v[set_number][i].valid){
                            pos = i;
                            break;
                        }
                    }
                    // adding first element to low priority 
                    vector<cell> temp2 = v[set_number];
                    temp2[pos] = add;
                    v[set_number] = temp2;
                    //all cases of write are handled
                    return;
                }

            }

        }
    }
    return;
}

vector<string> get_input(string input){
    vector<string> res;
    stringstream ss(input);
	string tok;

    while(getline(ss, tok, ' ')){
        res.push_back(tok);
    }
    
    res[0] = res[0].substr(0, res[0].size() - 1);
    if(res.size() == 3) res[1] = res[1].substr(0, res[1].size() - 1);

    return res;
}

int main(){
    string filename = "input.txt";
    fstream file;

    file.open(filename);

    int cache_size;
    int block_size;
    int assoc;
    int T;

    file>>cache_size;
    file>>block_size;
    file>>assoc;
    file>>T;

    vector<int> main_memory(cache_size*5);
    for(int i = 0; i < cache_size*5; ++i){
        main_memory[i] = i;
    }

    int access_count = 0;
    int read_miss = 0;
    int read_hit = 0;
    int write_hit = 0;
    int write_miss = 0;

    string line;

    getline(file, line);

    vector< vector<cell> > cache(cache_size/(assoc*block_size), vector<cell> (assoc));

    for(int i = 0 ; i < (cache_size/(block_size*assoc)); ++i){
        for(int j = 0; j < assoc; ++j){
            cache[i][j].valid = false;
            cache[i][j].data = 0;
            cache[i][j].priority = 0;
            cache[i][j].tag = 0;
            cache[i][j].last_call = -1;
            cache[i][j].dirty_bit = 0;
        }
    }


    int count = 1;
    while(getline(file, line)){
        access_count++;
        cout<<"Process for Instruction count "<<count<<endl;
        vector<string> input = get_input(line);
        process(cache, input, main_memory, assoc, cache_size/(block_size*assoc), T, read_hit, read_miss, write_hit, write_miss);  
        print_cache(cache);      
        count++;
    }

    double hits = read_hit + write_hit;
    double hit_ratio = (hits)/(access_count);

    cout<<"Cache statistics:"<<endl;
    cout<<"Number of Accesses = "<<access_count<<endl;
    cout<<"Number of Reads = "<<(read_miss + read_hit)<<endl;
    cout<<"Number of Read Hits = "<<read_hit<<endl;
    cout<<"Number of Read Misses = "<<read_miss<<endl;
    cout<<"Number of Writes = "<<(write_miss + write_hit)<<endl;
    cout<<"Number of Write Hits = "<<write_hit<<endl;
    cout<<"Number of Write Misses = "<<write_miss<<endl;
    cout<<"Hit Ratio = "<<hit_ratio<<endl;

    return 0;
}
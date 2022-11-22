#include <bits/stdc++.h>
using namespace std;



void senderCheckSum(char* a, int packetSize);
void recieverCheckSum(char* a, int packetSize);

int main(){
	char a[10] = {'a', 'b', 'c', 'd'};
	int packetSize = 4;
	senderCheckSum(a, packetSize);
	packetSize = 4;
	cout << "we got here" << endl;
	recieverCheckSum(a, packetSize);
}

void senderCheckSum(char* a, int packetSize){
	int counter = packetSize;
	string binary = "";
	int sum = 0;
	for(int i = 0; i < packetSize; i++){
		int val = int(a[i]);
		cout << val << endl;
		while(val > 0){
			if(val % 2){
				binary.push_back('1');
				a[counter] = '1';
				counter += 1;
			} else {
				binary.push_back('0');
				a[counter] = '0';
				counter += 1;
			}
			val /= 2;
		}
		cout << binary << endl;
		sum += stoi(binary);
		cout << sum << endl;
		binary = "";
		cout << binary << endl;
	}
	
	int j = packetSize;
	while(sum > 0){
		a[j] = sum % 10;
		sum /= 10;
		j++;
	}
	
}


void recieverCheckSum(char* a, int packetSize){
	int sum = 0;
	string binary = "";
	cout << packetSize << endl;
	for(int i = 0; i < packetSize; i++){
		if(i < packetSize){
		int val = int(a[i]);
		cout << val << endl;
		while(val > 0){
			if(val % 2){
				binary.push_back('1');
			} else {
				binary.push_back('0');
			}
			val /= 2;
		}
		sum += stoi(binary);
		cout << sum << endl;
		binary = "";
		cout << i << endl;
		}
	}
	
	string reCheck = "";
	cout << "we get here?" << endl;
	for(int j = packetSize; j < packetSize + 6; j++){
	cout << "a[" << j << "} = " << a[j] << endl;
		reCheck += a[j];
	}
	
	int check = stoi(reCheck);
	
	if(check = sum){
		cout << "Checksum OK" << endl;
	} else {
		cout << "Checksum failed" << endl;
	}
}
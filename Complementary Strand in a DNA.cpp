#include <iostream>
using namespace std;

int main() {
	int t,n;
	cin>>t;
	for(int i=1;i<=t;i++){
	    cin>>n;
	    char s[n];
	    for(int k=1;k<=n;k++){
	        cin>>s[k];
	        if(s[k]=='A'){
	            s[k]='T';     
	        }
	        else if(s[k]=='T'){
	            s[k]='A';     
	        }
	        else if(s[k]=='C'){
	            s[k]='G';     
	        }
	        else if(s[k]=='G'){
	            s[k]='C';     
	        }
	        cout<<s[k];
	    }
	    cout<<"\n";
	    
	}
	return 0;
}

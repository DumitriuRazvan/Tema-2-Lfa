#include <iostream>
#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <cstdlib>
/*ab-
11
0
1
10
13
0 1 -
1 2 -
1 4 -
2 3 a
4 5 b
3 6 -
5 6 -
6 1 -
0 7 -
7 8 a
8 9 b
9 10 b*/
using namespace std;

int **graf; // matricea grafului asociat automatului
string **tranz; // matrice de caractere care retine cu ce litere se poate trece dintr o stare in alta
int *fin; // functie caracteristica pentru starile finale
char alfabet[10]; // sir care retine alfabetul
int stIn; // starea initiala
int nrSt; // numarul de stari/noduri in graf
string *inch; // vector de string pentru lambda inchideri
string *lambdaTranz; // vector de string pt tranz lambda
vector<string> stDfa;

void citire(int **&g, string **&t, int *&fin, int& q, int& nrSt,char alf[], string *&inch)
{
//    Citirea se face din fisier in urmatoarea ordine:
//        -alfabet
//        -numarul de stari
//        -starea initiala
//        -numarul de stari finale si acestea
//        -numarul de tranzitii si acestea

    ifstream f("lnfa.txt");
    f >> alf;
    f >> nrSt;
    f >> q;

    inch = new string[nrSt];
    lambdaTranz = new string[nrSt];
    for(int i = 0; i < nrSt; i++)
    {
        inch[i] = "";
        lambdaTranz[i] = "";
    }


    int nrStFin;
    f >> nrStFin;
    fin = new int[nrSt]();
    for(int i = 0; i < nrStFin; i++)
    {
        int x;
        f >> x;
        fin[x] = 1;
    }

    //initializare matrice
    g = new int*[nrSt];
    t = new string*[nrSt];
    for(int i = 0; i < nrSt; i++)
    {
        g[i] = new int[nrSt]();
        t[i] = new string[nrSt]();
    }

    int nrTranz;
    f >> nrTranz;
    char lambda = '-';
    for(int i = 0; i < nrTranz; i++)
    {
        int x, y;
        char a;
        f >> x >> y >> a;
        g[x][y]++;
        t[x][y].push_back(a);
        if(a == lambda)
        {
            char y_char = y + '0';
            lambdaTranz[x].push_back(y_char);
        }
    }

    f.close();

}

//functie care cauta un char intr un string- daca face bucla
bool esteInInchidere(string close, string s)
{
    for(unsigned int i = 0; i < close.length(); i++)
    {
        if(close.compare(i, s.size(), s, 0, s.size()) == 0) //compara fiecare lit din inchid cu s
            return true;
    }
    return false;
}

void  lambdaInch(int stSursa, int stare)
{
    string st = to_string(stare); // converteste int in char
    inch[stSursa].append(st); // face push starii curente la inch
    inch[stSursa].push_back(',');
    string tr = lambdaTranz[stare];
    for(unsigned int i = 0; i < tr.length(); i++)
    {
        //trecem prin toate starile prin care putem cu lambda
        string s = string(1, tr[i]); //tr[i] e char, il convertim in string
        int s2 = tr[i] - '0'; //deoarece fct noastra acc un int il convertim in int
        if(esteInInchidere(inch[stSursa], s)==0)
            lambdaInch(stSursa, s2);
    }
}

//verifica daca starea este stare a dfa ului
bool esteStare(string stare)
{
    for(unsigned int i = 0; i < stDfa.size(); i++)
        if(stDfa[i].compare(stare) == 0)
            return true;
    return false;
}

//returneaza starile in care ne putem muta cu simbolul s din starea actuala
string tranzitii(int stare, char s)
{
    string tr = "";
    for(int i = 0; i < nrSt; i++)
    {
        string simb = tranz[stare][i];
        for(unsigned int j = 0; j < simb.length(); j++)
            if(s == simb[j])
            {
                tr.append(to_string(i));
                tr.push_back(',');
            }
    }
    return tr;
}

//stergem repetitiile din string
string deleteRep(string s)
{
    vector<int> stariPrezente;
    stariPrezente.resize(nrSt, 0); // vector de marime n cu valori 0, 0 nu e prezent, 1 e prezent

    for(unsigned int i = 0; i < s.length(); i++)
    {
        int stare = 0;
        if(i == 0)
        {
            // suntem pe prima pozitie
            if(s[i + 1] == ',')
            {
                // o singura cifra
                stare = s[i] - '0'; // convertim la int
            }
            else if(s[i + 2] == ',')
            {
                // doua cifre, facem un nr din doua cifre
                stare = s[i] - '0';
                stare = stare * 10 + (s[i + 1] - '0');
            }
        }
        else
        {
            if(s[i - 1] == ',' && s[i + 1] == ',')
            {
                // o cifra
                stare = s[i] - '0';
            }
            else if(s[i - 1] == ',' && s[i + 2] == ',')
            {
                // doua cifre
                stare = s[i] - '0';
                stare = stare * 10 + (s[i + 1] - '0');
            }
        }
        if(s[i] != ',')
            stariPrezente[stare] = 1;
    }

    string sir = "";
    for(unsigned int i = 0; i < stariPrezente.size(); i++)
    {
        if(stariPrezente[i] == 1)
        {
            sir.append(to_string(i));
            sir.push_back(',');
        }
    }
    return sir;
}

//functia care transforma nfa to dfa
void nfaToDfa(string stCur)
{
    string *stariNoi = new string[strlen(alfabet) - 1]();

    for(unsigned int i = 0; i < strlen(alfabet) - 1; i++)
    {
        // pentru fiecare simbol cu exceptia lui lambda
        for(unsigned int j = 0; j < stCur.length() - 1; j++)
        {
            int pozCur;
            string stari;
            if(j != 0) //verific sa nu fie prima pozitie din string
            {
                if(stCur[j - 1] == ',' && stCur[j + 1] == ',')
                {
                    // starea e o singura cifra
                    pozCur = stCur[j] - '0';
                    stari = tranzitii(pozCur, alfabet[i]);
                }
                else if(stCur[j - 1] == ',' && stCur[j + 2] == ',')
                {
                    // starea e din doua cifre
                    string stareCurenta; // pun ambele cifre intr un string
                    stareCurenta.push_back(stCur[j]);
                    stareCurenta.push_back(stCur[j + 1]);
                    pozCur = atoi(stareCurenta.c_str()); // convertesc stringul la int
                    stari = tranzitii(pozCur, alfabet[i]);
                }
            }
            else
            {
                // suntem pe prima pozitie
                if(stCur[j + 1] == ',')
                {
                    pozCur = stCur[j] - '0';
                    stari = tranzitii(pozCur, alfabet[i]);
                }
                else if(stCur[j + 2] == ',')
                {
                    string stareCurenta; // pun ambele cifre intr un string
                    stareCurenta.push_back(stCur[j]);
                    stareCurenta.push_back(stCur[j + 1]);
                    pozCur = atoi(stareCurenta.c_str()); // convertesc stringul la int
                    stari = tranzitii(pozCur, alfabet[i]);
                }
            }

            for(unsigned int k = 0; k < stari.length(); k++)
            {
                int stare;
                if(k == 0)
                {
                    // pe prima pozitie
                    if(stari[k + 1] == ',')
                    {
                        // e o singura cifra
                        stare = stari[k] - '0';
                    }
                    else if(stari[k + 2] == ',')
                    {
                        // doua cifre
                        stare = stari[k] - '0';
                        stare = stare * 10 + (stari[k + 1] - '0');
                    }
                }
                else
                {
                    if(stari[k - 1] == ',' && stari[k + 1])
                    {
                        stare = stari[k] - '0';
                    }
                    else if(stari[k - 1] == ',' && stari[k + 2] == ',')
                    {
                        stare = stari[k] - '0';
                        stare = stare * 10 + (stari[k + 1] - '0');
                    }
                }
                if(stari[k] != ',')
                    stariNoi[i].append(inch[stare]);
            }
            stariNoi[i] = deleteRep(stariNoi[i]);
        }
    }

    for(unsigned int i = 0; i < strlen(alfabet) - 1; i++)
    {
        if(stariNoi[i]=="")
            stariNoi[i]="99999";
        cout<<"De la starea "<<stCur<<" la "<<stariNoi[i]<<" cu "<<alfabet[i]<<endl;
        if(esteStare(stariNoi[i])==0 && (stariNoi[i].compare("") != 0))
        {
            stDfa.push_back(stariNoi[i]);
            nfaToDfa(stariNoi[i]);
        }
    }
}

int main()
{
    citire(graf, tranz, fin, stIn, nrSt, alfabet, inch);

    //fac lambda inchiderile pt toate starile
    for(int i = 0; i < nrSt; i++)
    {
        lambdaInch(i, i);
    }
//introducem starea initiala in dfa si de la ea plecam
    stDfa.push_back(inch[0]);
    nfaToDfa(inch[0]);
//    cout<<"Inchiderile sunt:"<<endl;
//    for(int i=0;i<nrSt;i++)
//        cout<<inch[i]<<endl;
    cout<<"Starea initiala:"<<stDfa[0]<<endl;
    cout << "Stari finale: ";
    for( int j=0; j<stDfa.size(); j++)
    {
        string s=stDfa[j];
        for(unsigned int i = 0; i < nrSt; i++) // trec prin toate nodurile
            if(fin[i] == 1 && esteInInchidere(s, to_string(i))) // daca starea e finala si se gaseste in starea s, atunci o afisez
                cout << s << " ";
    }

    return 0;
}

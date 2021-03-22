#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string> 
#include <algorithm>
#include <unordered_map>
#include <stack>
#include <regex>
#include <set>

using namespace std;

vector<string> split(const string& str, const string& delim)
{
	vector<string> tokens;
	size_t prev = 0, pos = 0;
	do
	{
		pos = str.find(delim, prev);
		if (pos == string::npos)
		{
			pos = str.length();
		}
		string token = str.substr(prev, pos - prev);
		if (!token.empty())
		{
			tokens.push_back(token);
		}
		prev = pos + delim.length();
	} while (pos < str.length() && prev < str.length());

	return tokens;
}

struct State
{
    vector<pair<string, string>> reglas;
    bool isTerminal;
    unordered_map<string, State*> transiciones;
    unordered_map<string, State*> llegada;
};

struct AFD
{   
    int cont  = 0;
    int cont2 = 0;
    unordered_map<int, State*> states;
};


void swap(vector<string> &aux, int i)
{
    string temp = aux[i];
    aux[i] = aux[i+1];
    aux[i+1] = temp;    
}


class ReadGrammar
{
    private:
        ifstream file;
        set<string> nonTerminals;
        set<string> terminals;
        set<string> universe;
        set<string> rightToPoint;
        set<pair<string, string>> pointsLeftAndRight;

        AFD* afd = new AFD();

        void fillPointRights(State *currentState)
        {
            for (int i = 0; i < currentState->reglas.size(); ++i)
            {
                vector<string> tempE = split(currentState->reglas[i].second, " ");
                for (int j = 0; j < tempE.size(); ++j)
                {
                    if (tempE[j] == ".")
                    {
                        rightToPoint.emplace(tempE[j+1]);
                    }
                }
            }            
        }

        void fillPointLeftAndRight(State *currentState)
        {
            for (int i = 0; i < currentState->reglas.size(); ++i)
            {
                vector<string> tempE = split(currentState->reglas[i].second, " ");
                for (int j = 0; j < tempE.size(); ++j)
                {
                    if (tempE[j] == ".")
                    {
                        if (j + 1 < tempE.size())
                        {
                            pointsLeftAndRight.emplace(make_pair(tempE[j-1], tempE[j+1]));
                        }
                        else
                        {
                            pointsLeftAndRight.emplace(make_pair(tempE[j-1], ""));
                        }
                    }
                }
            }            
        }

        pair<string, string> movePoint(int state, int rule)
        {
            auto aux = split(afd->states[state]->reglas[rule].second, " ");
            string cadena = "";

            for (int i = 0; i < aux.size() - 1; ++i)
            {
                if (aux[i] == ".")
                {
                    swap(aux, i);
                    break;
                }
            }

            for (int i = 0; i < aux.size(); ++i)
            {
                cadena = cadena + aux[i] + " ";
            }

            return make_pair(afd->states[state]->reglas[rule].first, cadena);
        }

    public:       
        ReadGrammar() {};
        ReadGrammar(string nombre) 
        {
            readGrammar(nombre);
        };

        void readGrammar(string nombre) 
        {
            vector<pair<string, string>> vec;
            vector<pair<string, string>> vecFinal;
            file.open(nombre);

            if (file.is_open())
            {
                while(!file.eof())
                {
                    string line;
                    getline(file,line);
                    if (line == "X") break;
                    line = line + "|";
                    auto sepFlecha = split(line, " -> ");
                    auto sepOr = split(sepFlecha[1], "|");
                    for (int i = 0; i < sepOr.size() - 1; ++i)
                    {
                        vec.push_back(make_pair(sepFlecha[0], sepOr[i]));
                        nonTerminals.emplace(sepFlecha[0]);

                        auto sepEspacio = split(sepOr[i], " ");
                        for (int j = 0; j < sepEspacio.size(); ++j)
                        {
                            universe.emplace(sepEspacio[j]);
                        }
                    }
                }

                set_difference(universe.begin(), universe.end(), nonTerminals.begin(), nonTerminals.end(), 
                inserter(terminals, terminals.end()));
            }

            for (int i = 0; i < vec.size(); ++i)
            {
                auto sepEspacio = split(vec[i].second, " ");
   
                for (int j = 0; j < sepEspacio.size(); ++j)
                {
                    string aux = "";
                    for (int k = 0; k < j; ++k)
                    {
                        aux = aux + " " + sepEspacio[k];
                    }
                    aux = aux + ". " + sepEspacio[j];
                    for (int k = j + 1; k < sepEspacio.size(); ++k)
                    {
                        aux = aux + " " + sepEspacio[k];
                    }
                    // Puntos intercalados
                    vecFinal.push_back(make_pair(vec[i].first,aux));
                 }
                 // Punto final
                 vecFinal.push_back(make_pair(vec[i].first, vec[i].second + "."));
            }

            // cout << "RULES\n";
            vector<pair<string, string>> startRules;
            for (size_t i = 0; i < vecFinal.size(); i++)
            {
                if (vecFinal[i].second[0] == '.')
                {
                    startRules.push_back(make_pair(vecFinal[i].first, vecFinal[i].second));
                }
                // cout<<vecFinal[i].first<<" -> "<<vecFinal[i].second<<endl;
            }
            // SE CREA EL ESTADO 0
            afd->states[afd->cont++] = new State{startRules, false, unordered_map<string, State*>(), unordered_map<string, State*>() };
            
            // auto nextRule = movePoint(0, 40);

            // cout << nextRule.first << " -> "<< nextRule.second << endl;
            // cout << endl;

            // vector<pair<string, string>> reglas
            // .first antes de la flecha
            // .second después de la flecha (ejemplo . Z1 Z3)
            auto currentState = afd->states[0];
            fillPointRights(currentState);

            for (auto it : rightToPoint)
            {
                vector<pair<string, string>> nuevasReglas;
                for (int i = 0; i < currentState->reglas.size(); ++i)
                {
                    vector<string> tempE = split(currentState->reglas[i].second, " ");

                    for (int j = 0; j < tempE.size(); ++j)
                    {
                        if (tempE[j] == "." && tempE[j + 1] == it)
                        {
                            nuevasReglas.push_back(movePoint(0, i));    
                        }
                    }
                }
                // Para que un estado sea terminal
                // debe existir una sola regla y que esta termine en punto
                // SE CREA EL ESTADO 1
                afd->states[afd->cont] = new State{nuevasReglas, 
                nuevasReglas.size() == 1 && split(nuevasReglas[0].second, " ").back() == ".", 
                unordered_map<string, State*>(), unordered_map<string, State*>() };

                // cout << it << endl;
                afd->states[afd->cont]->llegada[it] = afd->states[0];
                afd->states[0]->transiciones[it] = afd->states[afd->cont];
                afd->cont++;
            }

            // Iterar sobre los nuevos estados
            for (int i = 1; i < afd->cont; ++i)
            {
                currentState = afd->states[i];
                // creo que no se puede set de pairs, ya luego cambiamos eso voy a comer
                pointsLeftAndRight.clear();
                // Llenar right to points después de la movida.
                fillPointLeftAndRight(currentState);

                // Itero sobre los nuevos que están a la derecha del punto
                for (auto it : pointsLeftAndRight)
                {
                    // Si el nuevo que está a la derecha del punto 
                    // es un no terminal.
                    if (it.second != "")
                    {
                        if (nonTerminals.find(it.second) != nonTerminals.end())
                        {
                            // Z3 -> Z8 . Z6
                            // it.first = Z8, it.second = Z6
                            auto stateFrom = currentState->llegada[it.first];

                            for (int j = 0; j < stateFrom->reglas.size(); ++j)
                            {
                                // Encuentro las reglas que estaba buscando
                                // en el estado de donde vine
                                if (stateFrom->reglas[j].first == it.second)
                                {
                                    // Añado esas reglas al currentState
                                    currentState->reglas.push_back(stateFrom->reglas[j]);
                                }
                            }
                        }
                    }
                }
            }

            for (auto it2 : afd->states)
            {
                cout << "Estado: " << it2.first << endl;
                for (auto it : it2.second->reglas)
                {
                    cout << it.first << " -> " << it.second << endl;
                }
                cout << endl << endl;
            }
            

            /*
            // for (int i = 0; i < afd->states[0]->reglas.size(); ++i)
            // {
            //     afd->state[i].second;
            // }
            // afd->states[afd->cont++]

            // cout << "START RULES\n";
            // for (auto& it: afd->states[0]->reglas) 
            // {
            //     cout << it.first << " -> " << it.second << endl;
            // }

            // cout << "NON TERMINALS\n";
            // for (auto it : nonTerminals)
            // {
            //     cout << it << endl;
            // }
            
            // cout << "TERMINALS\n";
            // for (auto it : terminals)
            // {
            //     cout << it << endl;
            // }*/

            file.close();
        };
};


int main()
{
    ReadGrammar rg("leerGramatica.txt");
};
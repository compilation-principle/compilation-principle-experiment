#pragma warning (disable:4996)
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <cstring>
#include <string>
#include <cstdio>
#include <cmath>
#include <vector>
#include <stack>
#include <queue>
#include <map>
#include <set>
#define inf 0X3f3f3f3f
using namespace std;
typedef long long ll;
typedef unsigned long long ull;

/* ���ս����A~Z(') */
/* ����'�ļǺ� ���ս����string���ͱ�ʾ */
/* �ս�����������ַ� */
/* ���ַ����ţ�'\0' */

vector<string> split(const string& s, const string& seperator) {
	vector<string> result;
	typedef string::size_type string_size;
	string_size i = 0;

	if (s == "��") {
		result.push_back(s);
		return result;
	}

	while (i != s.size()) {
		//�ҵ��ַ������׸������ڷָ�������ĸ��
		int flag = 0;
		while (i != s.size() && flag == 0) {
			flag = 1;
			for (string_size x = 0; x < seperator.size(); ++x)
				if (s[i] == seperator[x]) {
					++i;
					flag = 0;
					break;
				}
		}
		//�ҵ���һ���ָ������������ָ���֮����ַ���ȡ����
		flag = 0;
		string_size j = i;
		while (j != s.size() && flag == 0) {
			for (string_size x = 0; x < seperator.size(); ++x)
				if (s[j] == seperator[x]) {
					flag = 1;
					break;
				}
			if (flag == 0)
				++j;
		}
		if (i != j) {
			result.push_back(s.substr(i, j - i));
			i = j;
		}
	}
	return result;
}

int phaseStringLengthFromBeginning(const string& str1, const string& str2) {
	int len = min(str1.length(), str2.length());
	for (int i = 0; i < len; i++) {
		if (str1[i] != str2[i])
			return i;
	}
	return len;
}

typedef struct Grammar {//�ķ�
	set<string> terminator;
	set<string> nonterminal;
	map<string, vector<string>> production;//����ʽ P -> ��... 
	string start;
}grammar;

class GrammarManager {
private:
	grammar gram;
	string language;
	map<string, set<string>> FIRST;// FIRST(��) = {a, ... }
	map<string, set<string>> FOLLOW;// FOLLOW(A) = {a, ... }
	map<pair<string, string>, string> PredictiveTable;// M[A, a] = ��
	queue<pair<pair<string, string>, string> > PredictiveProcess;//ջ��ʣ�മ������ʽ/ƥ��

public:
	void inputGrammar();
	void inputLanguage();//ޱ
	vector<string> splitTerminatorAndNonterminal(string str);
	void buildTerminator();
	void buildNonterminal();
	set<string> buildFIRSTofNonterminal(string str);
	void buildFIRST();
	set<string> buildFOLLOWTofNonterminal(string str);
	void buildFOLLOW();
	void buildPredictiveTable();
	bool isLL1();
	bool hasLeftCommonFactor();
	bool hasLeftRecursive();
	void EliminateTheLeftCommonFactor();
	bool hasDirectLeftRecursionofNonTerminal(string str);
	void EliminateTheDirectLeftRecursion();
	void EliminateTheUndirectLeftRecursion();
	void CutUselessProductions();
	void EliminateTheLeftRecursion();
	bool analyzation();//ޱ
	void outputPrediction();
	void outputAnalyzation();//ޱ
};

void GrammarManager::inputGrammar() {
	cout << "Input the grammar productions: " << endl;
	//cout << "Input ctrl+Z ending the input" << endl;
	cout << "Input \"EOF\" ending the input" << endl;
	string str;
	while (cin >> str) {
		if (str == "EOF")
			break;
		vector<string> vec = split(str, "->");
		string c = vec.front();//���ս��
		if (!gram.start.length())
			gram.start = c;
		vec = split(vec.back(), "|");//����ʽ
		for (int i = 0; i < vec.size(); i++)
			gram.production[c].push_back(vec[i]);
	}
}

void GrammarManager::inputLanguage() {
	string str = "";
	cout << "Input the string to be analyzed:" << endl;
	cout << "Input ctrl+Z ending the input" << endl;
	while (cin >> str) {
		language = str + "#";//��ĩβ����#��ʾ������
		if (analyzation())//�����ɹ�
			outputAnalyzation();
		else
			cout << "����������������룡" << endl;
	}
}

void GrammarManager::outputPrediction() {
	cout << "Grammar:" << endl;
	for (map<string, vector<string>>::iterator iter = gram.production.begin(); iter != gram.production.end(); iter++) {
		cout << (*iter).first << "->";
		for (int i = 0; i < (*iter).second.size(); i++) {
			cout << (*iter).second[i];
			if (i != (*iter).second.size() - 1)
				cout << '|';
		}
		cout << endl;
	}
	cout << "FIRST set:" << endl;
	for (map<string, set<string>>::iterator iter = FIRST.begin(); iter != FIRST.end(); iter++) {
		cout << "First(" << (*iter).first << ")={";
		int siz = (*iter).second.size();
		int cnt = 0;
		for (set<string>::iterator it = (*iter).second.begin(); it != (*iter).second.end(); it++) {
			cout << *it;
			if ((++cnt) < siz)
				cout << ", ";
		}
		cout << " }" << endl;
	}
	cout << "FOLLOW set:" << endl;
	for (map<string, set<string>>::iterator iter = FOLLOW.begin(); iter != FOLLOW.end(); iter++) {
		cout << "Follow(" << (*iter).first << ")={ ";
		int siz = (*iter).second.size();
		int cnt = 0;
		for (set<string>::iterator it = (*iter).second.begin(); it != (*iter).second.end(); it++) {
			cout << *it;
			if ((++cnt) < siz)
				cout << ", ";
		}
		cout << " }" << endl;
	}
}

void GrammarManager::outputAnalyzation() {
	int cnt = 1;
	cout << setw(6) << left << "����";
	cout << setw(15) << left << "����ջ";
	cout << setw(20) << right << "ʣ�����봮";
	cout << setw(30) << right << "�Ƶ����ò���ʽ��ƥ��" << endl;
	while (!PredictiveProcess.empty()) {
		cout << setw(6) << left << cnt++;
		cout << setw(15) << left << PredictiveProcess.front().first.first;
		cout << setw(20) << right << PredictiveProcess.front().first.second;
		cout << setw(30) << right << PredictiveProcess.front().second << endl;
		PredictiveProcess.pop();
	}
}

bool GrammarManager::analyzation() {
	string strStack = "#" + gram.start;	//���ַ�����ʾ��ǰջ
	cout << strStack << endl;

	while (!strStack.empty()) {
		string topString = "", firstTerminal = "";
		topString += strStack[strStack.size() - 1];	//ջ��
		firstTerminal  += language[0];	//language�еĵ�һ���ս��

		if (topString == "#" && firstTerminal == "#") {
			PredictiveProcess.push(make_pair(make_pair(strStack, language), "����"));
			break;
		}

		if (topString.length() == 1 && topString == firstTerminal)	//ƥ��
		{
			PredictiveProcess.push(make_pair(make_pair(strStack, language), "\"" + topString + "\"ƥ��"));
			strStack.erase(strStack.end() - 1);	//����ջ��Ԫ��
			language.erase(language.begin());	//ɾ�����봮��Ԫ��
		}
		else {
			string production = PredictiveTable[make_pair(topString, firstTerminal)];	//����Ԥ���
			if (production != "") {	//��Ϊ��
				PredictiveProcess.push(make_pair(make_pair(strStack, language), production));
				strStack.erase(strStack.end() - 1);	//����ջ��Ԫ��

				//������ʽ���Ų���ջ��
				int productionLength = production.length();
				for (int i = productionLength - 1; i >= 0; i--) {
					if (i >= 1 && production[i] == '>' && production[i - 1] == '-')//����->���˳�
						break;
					strStack += production[i];	//����ջ��
				}
			}
			else {	//��������
				return false;
			}
		}
	}
	return true;
}


vector<string> GrammarManager::splitTerminatorAndNonterminal(string str) {
	vector<string> res;
	bool flag = 0;//ǰһ���ַ��Ƿ����ڷ��ս��
	int pos = 0;
	for (int i = 0; i < str.length(); i++) {
		if ((str[i] >= 'A' && str[i] <= 'Z') || str[i] == '\'') {
			if (!flag) {
				pos = i;
				flag = 1;
			}
			else if (str[i] >= 'A' && str[i] <= 'Z') {
				res.push_back(str.substr(pos, i - pos));
				pos = i;
			}
		}
		else {
			if (!flag)
				res.push_back(string(1, str[i]));
			else {
				res.push_back(str.substr(pos, i - pos));
				res.push_back(string(1, str[i]));
				flag = 0;
			}
		}
	}
	if (flag)
		res.push_back(str.substr(pos, str.length() - pos));
	return res;
}

void GrammarManager::buildTerminator() {//�����ս������
	for (map<string, vector<string>>::iterator iter = gram.production.begin(); iter != gram.production.end(); iter++) {
		for (int i = 0; i < (*iter).second.size(); i++) {
			string tmps = (*iter).second[i];
			if (tmps == "��")
				continue;
			vector<string> vec = splitTerminatorAndNonterminal(tmps);
			for (int j = 0; j < vec.size(); j++) {
				if (vec[j].length() == 1 && (vec[j][0] < 'A' || vec[j][0] >'Z'))
					gram.terminator.insert(vec[j]);
			}
		}
	}
}

void GrammarManager::buildNonterminal() {//�������ս������
	for (map<string, vector<string>>::iterator iter = gram.production.begin(); iter != gram.production.end(); iter++) {
		string c = (*iter).first;
		gram.nonterminal.insert(c);
	}
}

/*������װ���һ���ս����FIRST���ϵĺ�����Ϊ�˿��Եݹ����*/
set<string> GrammarManager::buildFIRSTofNonterminal(string str) {//������ʽstr��first����
	set<string> first;//��ʼ���յ�first����
	if (str == "��") {
		first.insert("��");
		return first;
	}
	vector<string> vec = splitTerminatorAndNonterminal(str);
	for (int i = 0; i < vec.size(); i++) {//������ʽ��ÿһ���ַ�
		if (gram.terminator.count(vec[i])) {
			first.insert(vec[i]);//�ս������first
			return first;
		}
		else {
			bool flag = 0;
			for (int j = 0; j < gram.production[vec[i]].size(); j++) {//���ս����ÿһ������ʽ
				string tmps = gram.production[vec[i]][j];
				set<string> add;
				if (FIRST.count(tmps))
					add = FIRST[tmps];
				else
					add = buildFIRSTofNonterminal(tmps);//�ݹ����
				for (set<string>::iterator iter = add.begin(); iter != add.end(); iter++) {
					if (*iter != "��")
						first.insert(*iter);//���ս����first����first
				}
				if (add.count("��"))//�����մ�
					flag = 1;
			}
			if (!flag)
				return first;
		}
	}
	first.insert("��");
	return first;
}

void GrammarManager::buildFIRST() {
	for (map<string, vector<string>>::iterator iter = gram.production.begin(); iter != gram.production.end(); iter++) {//����ÿһ�����ս��
		for (int i = 0; i < (*iter).second.size(); i++) {
			string tmps = (*iter).second[i];//�Բ���ʽtmps
			FIRST[tmps] = buildFIRSTofNonterminal(tmps);
		}
	}
}

set<string> GrammarManager::buildFOLLOWTofNonterminal(string str) {//�����ս��str��follow����
	set<string> follow;//��ʼ���յ�follow����
	for (map<string, vector<string>>::iterator iter = gram.production.begin(); iter != gram.production.end(); iter++) {
		string c = (*iter).first;//��ÿһ�����ս��
		for (int i = 0; i < (*iter).second.size(); i++) {
			string tmps = (*iter).second[i];//��ÿһ������ʽtmps
			if (tmps == "��")
				continue;
			vector<string> vec = splitTerminatorAndNonterminal(tmps);
			for (int j = 0; j < vec.size(); j++) {
				if (vec[j] != str)//�ҵ�str���ֹ��Ĳ���ʽ
					continue;
				//֮����ַ������Ƶ����մ� �� ֮��û���ַ� ������ս��c��follow
				bool flag = 0;
				if (j < vec.size() - 1) {
					if (gram.nonterminal.count(vec[j + 1])) {
						for (int k = 0; k < gram.production[vec[j + 1]].size(); k++) {//���ս��vec[j]��ÿһ������ʽ
							string tmps2 = gram.production[vec[j + 1]][k];//���ս���Ĳ���ʽtmps2
							if (FIRST[tmps2].count("��")) {//������first�����Ƿ�����մ�
								flag = 1;
								break;
							}
							for (set<string>::iterator it = FIRST[tmps2].begin(); it != FIRST[tmps2].end(); it++) {
								if (*it != "��")//���ս���Ĳ���ʽ��first���ϳ��˿մ�����follow����
									follow.insert(*it);
							}
						}
					}
					else {//֮����ս������follow����
						follow.insert(vec[j + 1]);
					}
				}
				/*�ܷ��Ƶ����մ� ����ת��Ϊ���ս���Ĳ���ʽ��first�Ƿ������*/
				if (flag || j == vec.size() - 1) {//���ս���Ƶ����մ�
					set<string> add;
					if (!FOLLOW.count(c))
						add = buildFOLLOWTofNonterminal((*iter).first);//�ݹ����
					else
						add = FOLLOW[c];
					for (set<string>::iterator it = add.begin(); it != add.end(); it++) {
						follow.insert(*it);
					}
				}
			}
		}
	}
	return follow;
}

void GrammarManager::buildFOLLOW() {
	for (map<string, vector<string>>::iterator iter = gram.production.begin(); iter != gram.production.end(); iter++) {
		string c = (*iter).first;
		FOLLOW[c] = buildFOLLOWTofNonterminal(c);
		if (c == gram.start)
			FOLLOW[c].insert("#");
	}
}

void GrammarManager::buildPredictiveTable() {
	for (map<string, vector<string>>::iterator iter = gram.production.begin(); iter != gram.production.end(); iter++) {//��ÿһ�����ս��
		string c = (*iter).first;//����ÿһ�����ս��c
		bool flag = 0;
		for (int i = 0; i < (*iter).second.size(); i++) {
			string tmps = (*iter).second[i];//����ÿһ������ʽ
			if (tmps != "��")
				for (set<string>::iterator it = FIRST[tmps].begin(); it != FIRST[tmps].end(); it++) {
					PredictiveTable[make_pair(c, (*it))] = tmps;
				}
			if (FIRST[tmps].count("��"))//�������ʽ�����Ƶ����մ� first���ϰ����մ�
				flag = 1;
		}
		if (flag) {//������ս�������Ƶ����մ�
			for (set<string>::iterator it = FOLLOW[c].begin(); it != FOLLOW[c].end(); it++) {
				PredictiveTable[make_pair(c, (*it))] = "��";
			}
		}
		for (set<string>::iterator it = gram.terminator.begin(); it != gram.terminator.end(); it++) {
			if (!PredictiveTable.count(make_pair(c, (*it))))
				PredictiveTable[make_pair(c, (*it))] = "error";//��ǳ����־
		}
	}
}

bool GrammarManager::isLL1() {//�ж�LL(1)�ķ�
	buildTerminator();
	buildNonterminal();
	if (hasLeftCommonFactor()) {
		cout << "Grammar has left common factor!" << endl;
		EliminateTheLeftCommonFactor();
		cout << "The Left Common Factor Eliminated!" << endl;
	}
	if (hasLeftRecursive()) {
		cout << "Grammar has left recursive!" << endl;
		EliminateTheLeftRecursion();
		cout << "The Left recursive Eliminated!" << endl;
	}
	buildFIRST();
	buildFOLLOW();
	set<string> check;//����Ƿ����ظ�
	for (map<string, vector<string>>::iterator iter = gram.production.begin(); iter != gram.production.end(); iter++) {
		string c = (*iter).first;//��ÿһ�����ս��
		for (int i = 0; i < (*iter).second.size(); i++) {
			string tmps1 = (*iter).second[i];//����ʽ1
			//���ڷ��ս���Ĳ���ʽ��first���������ཻΪ��
			for (int j = 0; j < (*iter).second.size(); j++) {
				if (i == j)
					continue;
				string tmps2 = (*iter).second[j];//����ʽ2
				for (set<string>::iterator it = FIRST[tmps2].begin(); it != FIRST[tmps2].end(); it++) {
					if (*it != "��" && FIRST[tmps1].count(*it))//�������ʽ2��first�е�Ԫ�س����ڲ���ʽ1��first��
						return false;
				}
			}
			if (FIRST[tmps1].count("��")) {//������Ƶ����մ� 
				//follow���������಻Ϊ�յ�����ʽ��first���ϲ��ཻ
				for (int j = 0; j < (*iter).second.size(); j++) {
					if (i == j)
						continue;
					string tmps2 = (*iter).second[j];//����ʽ2
					for (set<string>::iterator it = FIRST[tmps2].begin(); it != FIRST[tmps2].end(); it++) {
						if (FOLLOW[c].count(*it))//�������ʽ2��first�е�Ԫ�س����ڲ���ʽ1��follow��
							return false;
					}
				}
			}
		}
	}
	return true;
}

/*������ս��c�Ĳ���ʽû����ͬ�������ַ� ��һ���������󹫹�����*/
bool GrammarManager::hasLeftCommonFactor() {
	for (set<string>::iterator iter = gram.nonterminal.begin(); iter != gram.nonterminal.end(); iter++) {
		string c = *iter;//���ڷ��ս��c
		set<string> check;
		bool flag = 0;
		for (int i = 0; i < gram.production[c].size(); i++) {
			string tmps = gram.production[c][i];//���ڲ���ʽtmps
			if (tmps == "��") {//����пմ�
				flag = 1;
				continue;
			}
			vector<string> vec = splitTerminatorAndNonterminal(tmps);
			check.insert(vec[0]);//������ʽ�ĵ�һ���ַ�����check
		}
		if (flag) {
			if (check.size() < gram.production[c].size() - 1)
				return true;
		}
		else if (check.size() < gram.production[c].size())
			return true;
	}
	return false;
}

/*��������ͼ �ڵ��Ƿ��ս�� �������ʽ��B��A��������ս�� ��һ��Aָ��B�ı�*/
/*�������ͼ���ڻ�·���ʾ������ݹ� ����(dfs)������ͼ�Ļ�·*/
bool GrammarManager::hasLeftRecursive() {
	stack<string> stk;
	stk.push(gram.start);
	set<string> vis;//�ѷ��ʹ��ķ��ս��
	while (stk.size()) {
		string now = stk.top();
		if (vis.count(now))//�����ǰ���ս���Ѿ������ʹ�
			return true;
		vis.insert(now);
		stk.pop();
		for (int i = 0; i < gram.production[now].size(); i++) {
			string tmps = gram.production[now][i];//����ÿһ������ʽ
			if (tmps == "��")
				continue;
			vector<string> vec = splitTerminatorAndNonterminal(tmps);
			if (gram.nonterminal.count(vec[0]))//������ʽΪ���ս���ĵ�һ���ַ�����ջ��
				stk.push(vec[0]);
		}
	}
	return false;
}

void GrammarManager::EliminateTheLeftCommonFactor() {
	string newc = "";
	while (hasLeftCommonFactor()) {//��������(�������ȸ��Ӷ���ͬ) ͬ�������ȳ��ֵ����ж�
		for (set<string>::iterator iter = gram.nonterminal.begin(); iter != gram.nonterminal.end(); iter++) {
			string c = *iter;
			if (newc == "")
				newc = c + '\'';
			else
				newc = newc + '\'';//�µķ��ս��
			int max_len = 0;//�󹫹����������
			set<string> store;//�洢������ȡ�󹫹����ӵĲ���ʽ
			string factor;//��������
			for (int i = 0; i < gram.production[c].size(); i++)
				for (int j = 0; j < gram.production[c].size(); j++) {
					if (j == i)
						continue;
					int len = phaseStringLengthFromBeginning(gram.production[c][i], gram.production[c][j]);
					if (max_len < len) {//��������������󹫹�����
						store.clear();
						factor = gram.production[c][i].substr(0, len);
						store.insert(gram.production[c][i]);
						store.insert(gram.production[c][j]);
						max_len = len;
					}
					else if (len == max_len) {//��ǰ���󹫹�����
						if (gram.production[c][j].substr(0, len) == factor)
							store.insert(gram.production[c][j]);
					}
				}
			vector<string> newpro;//�·��ս���Ĳ���ʽ
			vector<string> oripro;//ԭ���ս���Ĳ���ʽ
			for (int i = 0; i < gram.production[c].size(); i++) {
				string tmps = gram.production[c][i];
				if (store.count(tmps)) {
					if (tmps == factor)
						newpro.push_back("��");
					else
						newpro.push_back(tmps.substr(max_len));
				}
				else {
					oripro.push_back(tmps);
				}
			}
			oripro.push_back(factor + newc);
			gram.production[c].clear();
			gram.production[c] = oripro;
			gram.production[newc] = newpro;//������󹫹�����
		}
	}
	buildNonterminal();
}

bool GrammarManager::hasDirectLeftRecursionofNonTerminal(string str) {//���ڷ��ս��str
	set<string> check;
	for (int i = 0; i < gram.production[str].size(); i++) {
		string tmps = gram.production[str][i];
		if (tmps == "��")
			continue;
		vector<string> vec = splitTerminatorAndNonterminal(tmps);
		if (vec[0] == str)
			return true;
	}
	return false;
}

void GrammarManager::EliminateTheDirectLeftRecursion() {//����ֱ����ݹ�
	vector<string> oripro;//�洢ԭ����ʽ
	vector<string> newpro;//�洢�µĲ���ʽ
	for (set<string>::iterator iter = gram.nonterminal.begin(); iter != gram.nonterminal.end(); iter++) {
		string c = *iter;//��ÿһ�����ս��c
		string newc = c + '\'';//�µķ��ս��
		if (hasDirectLeftRecursionofNonTerminal(c)) {
			for (int i = 0; i < gram.production[c].size(); i++) {
				string tmps = gram.production[c][i];//��ÿһ������ʽtmps
				if (tmps == "��")
					continue;
				vector<string> vec = splitTerminatorAndNonterminal(tmps);
				if (vec[0] == c)
					newpro.push_back(tmps.substr(vec[0].length()) + newc);
				else {
					oripro.push_back(tmps + newc);
				}
			}
			newpro.push_back("��");
			gram.production[c].clear();
			gram.production[c] = oripro;
			gram.production[newc] = newpro;
		}
	}
	buildNonterminal();//����
}

void GrammarManager::EliminateTheUndirectLeftRecursion() {//���������ݹ�
	vector<string> order;//��һ����˳��洢���ս��(map����)
	for (set<string>::iterator iter = gram.nonterminal.begin(); iter != gram.nonterminal.end(); iter++) {
		order.push_back(*iter);
	}
	for (int i = 0; i < order.size(); i++) {
		for (int j = 0; j < i; j++) {
			vector<string> newpro;//�洢�µĲ���ʽ
			for (int k = 0; k < gram.production[order[i]].size(); k++) {//�����еĲ���ʽ
				string tmps = gram.production[order[i]][k];
				if (tmps == "��")
					continue;
				vector<string> vec = splitTerminatorAndNonterminal(tmps);
				if (vec[0] == order[j])//����
					for (int l = 0; l < gram.production[order[j]].size(); l++) {
						if (gram.production[order[j]][l] == "��")
							newpro.push_back(tmps.substr(vec[0].length()));
						else//����ƴ�Ӻ�Ĳ���ʽ
							newpro.push_back(gram.production[order[j]][l] + tmps.substr(vec[0].length()));
					}
				else//ԭ����ʽ
					newpro.push_back(order[i]);
			}
			gram.production[order[i]].clear();
			gram.production[order[i]] = newpro;
		}
	}
	EliminateTheDirectLeftRecursion();//����֮���ֱ����ݹ�
	CutUselessProductions();//������õĲ���ʽ
}

void GrammarManager::CutUselessProductions() {//ɾ�����õĲ���ʽ
	stack<string> stk;
	stk.push(gram.start);
	set<string> cnt;//��¼���ʹ��ķ��ս��
	while (stk.size()) {
		string now = stk.top();
		cnt.insert(now);
		stk.pop();
		for (int i = 0; i < gram.production[now].size(); i++) {
			string tmps = gram.production[now][i];//����ÿһ������ʽ
			vector<string> vec = splitTerminatorAndNonterminal(tmps);
			if (gram.nonterminal.count(vec[0]))
				stk.push(vec[0]);
		}
	}
	for (set<string>::iterator iter = gram.nonterminal.begin(); iter != gram.nonterminal.end(); iter++) {
		if (!cnt.count(*iter))//ɾ��δ���ֹ��ķ��ս���Ĳ���ʽ
			gram.production.erase(gram.production.find(*iter));
	}
}

void GrammarManager::EliminateTheLeftRecursion() {
	EliminateTheDirectLeftRecursion();
	EliminateTheUndirectLeftRecursion();
}

int main() {
	GrammarManager manager;

	manager.inputGrammar();
	while (!manager.isLL1()) {
		cout << "Not LL1 grammar! Input again!" << endl;
		manager.inputGrammar();
	}
	manager.outputPrediction();
	manager.buildPredictiveTable();
	manager.inputLanguage();

	return 0;
}

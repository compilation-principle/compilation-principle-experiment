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

/* 非终结符：A~Z(') */
/* 加入'的记号 非终结符用string类型表示 */
/* 终结符：其他的字符 */
/* 空字符串ε：'\0' */

vector<string> split(const string& s, const string& seperator) {
	vector<string> result;
	typedef string::size_type string_size;
	string_size i = 0;

	if (s == "ε") {
		result.push_back(s);
		return result;
	}

	while (i != s.size()) {
		//找到字符串中首个不等于分隔符的字母；
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
		//找到又一个分隔符，将两个分隔符之间的字符串取出；
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

typedef struct Grammar {//文法
	set<string> terminator;
	set<string> nonterminal;
	map<string, vector<string>> production;//产生式 P -> α... 
	string start;
}grammar;

class GrammarManager {
private:
	grammar gram;
	string language;
	map<string, set<string>> FIRST;// FIRST(α) = {a, ... }
	map<string, set<string>> FOLLOW;// FOLLOW(A) = {a, ... }
	map<pair<string, string>, string> PredictiveTable;// M[A, a] = α
	queue<pair<pair<string, string>, string> > PredictiveProcess;//栈，剩余串，产生式/匹配

public:
	void inputGrammar();
	void inputLanguage();//薇
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
	bool analyzation();//薇
	void outputPrediction();
	void outputAnalyzation();//薇
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
		string c = vec.front();//非终结符
		if (!gram.start.length())
			gram.start = c;
		vec = split(vec.back(), "|");//产生式
		for (int i = 0; i < vec.size(); i++)
			gram.production[c].push_back(vec[i]);
	}
}

void GrammarManager::inputLanguage() {
	string str = "";
	cout << "Input the string to be analyzed:" << endl;
	cout << "Input ctrl+Z ending the input" << endl;
	while (cin >> str) {
		language = str + "#";//在末尾加入#表示语句结束
		if (analyzation())//分析成功
			outputAnalyzation();
		else
			cout << "输入出错，请重新输入！" << endl;
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
	cout << setw(6) << left << "步骤";
	cout << setw(15) << left << "分析栈";
	cout << setw(20) << right << "剩余输入串";
	cout << setw(30) << right << "推导所用产生式或匹配" << endl;
	while (!PredictiveProcess.empty()) {
		cout << setw(6) << left << cnt++;
		cout << setw(15) << left << PredictiveProcess.front().first.first;
		cout << setw(20) << right << PredictiveProcess.front().first.second;
		cout << setw(30) << right << PredictiveProcess.front().second << endl;
		PredictiveProcess.pop();
	}
}

bool GrammarManager::analyzation() {
	string strStack = "#" + gram.start;	//用字符串表示当前栈
	cout << strStack << endl;

	while (!strStack.empty()) {
		string topString = "", firstTerminal = "";
		topString += strStack[strStack.size() - 1];	//栈顶
		firstTerminal  += language[0];	//language中的第一个终结符

		if (topString == "#" && firstTerminal == "#") {
			PredictiveProcess.push(make_pair(make_pair(strStack, language), "接受"));
			break;
		}

		if (topString.length() == 1 && topString == firstTerminal)	//匹配
		{
			PredictiveProcess.push(make_pair(make_pair(strStack, language), "\"" + topString + "\"匹配"));
			strStack.erase(strStack.end() - 1);	//弹出栈顶元素
			language.erase(language.begin());	//删除输入串首元素
		}
		else {
			string production = PredictiveTable[make_pair(topString, firstTerminal)];	//查找预测表
			if (production != "") {	//不为空
				PredictiveProcess.push(make_pair(make_pair(strStack, language), production));
				strStack.erase(strStack.end() - 1);	//弹出栈顶元素

				//将产生式倒着插入栈中
				int productionLength = production.length();
				for (int i = productionLength - 1; i >= 0; i--) {
					if (i >= 1 && production[i] == '>' && production[i - 1] == '-')//遇到->就退出
						break;
					strStack += production[i];	//插入栈首
				}
			}
			else {	//分析出错
				return false;
			}
		}
	}
	return true;
}


vector<string> GrammarManager::splitTerminatorAndNonterminal(string str) {
	vector<string> res;
	bool flag = 0;//前一个字符是否属于非终结符
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

void GrammarManager::buildTerminator() {//建立终结符集合
	for (map<string, vector<string>>::iterator iter = gram.production.begin(); iter != gram.production.end(); iter++) {
		for (int i = 0; i < (*iter).second.size(); i++) {
			string tmps = (*iter).second[i];
			if (tmps == "ε")
				continue;
			vector<string> vec = splitTerminatorAndNonterminal(tmps);
			for (int j = 0; j < vec.size(); j++) {
				if (vec[j].length() == 1 && (vec[j][0] < 'A' || vec[j][0] >'Z'))
					gram.terminator.insert(vec[j]);
			}
		}
	}
}

void GrammarManager::buildNonterminal() {//建立非终结符集合
	for (map<string, vector<string>>::iterator iter = gram.production.begin(); iter != gram.production.end(); iter++) {
		string c = (*iter).first;
		gram.nonterminal.insert(c);
	}
}

/*单独封装求解一个终结符的FIRST集合的函数是为了可以递归调用*/
set<string> GrammarManager::buildFIRSTofNonterminal(string str) {//求解产生式str的first集合
	set<string> first;//初始化空的first集合
	if (str == "ε") {
		first.insert("ε");
		return first;
	}
	vector<string> vec = splitTerminatorAndNonterminal(str);
	for (int i = 0; i < vec.size(); i++) {//对生成式的每一个字符
		if (gram.terminator.count(vec[i])) {
			first.insert(vec[i]);//终结符放入first
			return first;
		}
		else {
			bool flag = 0;
			for (int j = 0; j < gram.production[vec[i]].size(); j++) {//非终结符的每一个产生式
				string tmps = gram.production[vec[i]][j];
				set<string> add;
				if (FIRST.count(tmps))
					add = FIRST[tmps];
				else
					add = buildFIRSTofNonterminal(tmps);//递归求解
				for (set<string>::iterator iter = add.begin(); iter != add.end(); iter++) {
					if (*iter != "ε")
						first.insert(*iter);//非终结符的first放入first
				}
				if (add.count("ε"))//包含空串
					flag = 1;
			}
			if (!flag)
				return first;
		}
	}
	first.insert("ε");
	return first;
}

void GrammarManager::buildFIRST() {
	for (map<string, vector<string>>::iterator iter = gram.production.begin(); iter != gram.production.end(); iter++) {//对于每一个非终结符
		for (int i = 0; i < (*iter).second.size(); i++) {
			string tmps = (*iter).second[i];//对产生式tmps
			FIRST[tmps] = buildFIRSTofNonterminal(tmps);
		}
	}
}

set<string> GrammarManager::buildFOLLOWTofNonterminal(string str) {//求解非终结符str的follow集合
	set<string> follow;//初始化空的follow集合
	for (map<string, vector<string>>::iterator iter = gram.production.begin(); iter != gram.production.end(); iter++) {
		string c = (*iter).first;//对每一个非终结符
		for (int i = 0; i < (*iter).second.size(); i++) {
			string tmps = (*iter).second[i];//对每一个产生式tmps
			if (tmps == "ε")
				continue;
			vector<string> vec = splitTerminatorAndNonterminal(tmps);
			for (int j = 0; j < vec.size(); j++) {
				if (vec[j] != str)//找到str出现过的产生式
					continue;
				//之后的字符可以推导出空串 或 之后没有字符 加入非终结符c的follow
				bool flag = 0;
				if (j < vec.size() - 1) {
					if (gram.nonterminal.count(vec[j + 1])) {
						for (int k = 0; k < gram.production[vec[j + 1]].size(); k++) {//非终结符vec[j]的每一个产生式
							string tmps2 = gram.production[vec[j + 1]][k];//非终结符的产生式tmps2
							if (FIRST[tmps2].count("ε")) {//查找其first集合是否包含空串
								flag = 1;
								break;
							}
							for (set<string>::iterator it = FIRST[tmps2].begin(); it != FIRST[tmps2].end(); it++) {
								if (*it != "ε")//非终结符的产生式的first集合除了空串加入follow集合
									follow.insert(*it);
							}
						}
					}
					else {//之后的终结符加入follow集合
						follow.insert(vec[j + 1]);
					}
				}
				/*能否推导出空串 可以转换为非终结符的产生式的first是否包含ε*/
				if (flag || j == vec.size() - 1) {//非终结符推导至空串
					set<string> add;
					if (!FOLLOW.count(c))
						add = buildFOLLOWTofNonterminal((*iter).first);//递归求解
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
	for (map<string, vector<string>>::iterator iter = gram.production.begin(); iter != gram.production.end(); iter++) {//对每一个非终结符
		string c = (*iter).first;//对于每一个非终结符c
		bool flag = 0;
		for (int i = 0; i < (*iter).second.size(); i++) {
			string tmps = (*iter).second[i];//对于每一个产生式
			if (tmps != "ε")
				for (set<string>::iterator it = FIRST[tmps].begin(); it != FIRST[tmps].end(); it++) {
					PredictiveTable[make_pair(c, (*it))] = tmps;
				}
			if (FIRST[tmps].count("ε"))//如果产生式可以推导出空串 first集合包含空串
				flag = 1;
		}
		if (flag) {//如果非终结符可以推导出空串
			for (set<string>::iterator it = FOLLOW[c].begin(); it != FOLLOW[c].end(); it++) {
				PredictiveTable[make_pair(c, (*it))] = "ε";
			}
		}
		for (set<string>::iterator it = gram.terminator.begin(); it != gram.terminator.end(); it++) {
			if (!PredictiveTable.count(make_pair(c, (*it))))
				PredictiveTable[make_pair(c, (*it))] = "error";//标记出错标志
		}
	}
}

bool GrammarManager::isLL1() {//判断LL(1)文法
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
	set<string> check;//检查是否有重复
	for (map<string, vector<string>>::iterator iter = gram.production.begin(); iter != gram.production.end(); iter++) {
		string c = (*iter).first;//对每一个非终结符
		for (int i = 0; i < (*iter).second.size(); i++) {
			string tmps1 = (*iter).second[i];//产生式1
			//对于非终结符的产生式的first集合两两相交为空
			for (int j = 0; j < (*iter).second.size(); j++) {
				if (i == j)
					continue;
				string tmps2 = (*iter).second[j];//产生式2
				for (set<string>::iterator it = FIRST[tmps2].begin(); it != FIRST[tmps2].end(); it++) {
					if (*it != "ε" && FIRST[tmps1].count(*it))//如果产生式2的first中的元素出现在产生式1的first中
						return false;
				}
			}
			if (FIRST[tmps1].count("ε")) {//如果能推导出空串 
				//follow集合与其余不为空的生成式的first集合不相交
				for (int j = 0; j < (*iter).second.size(); j++) {
					if (i == j)
						continue;
					string tmps2 = (*iter).second[j];//产生式2
					for (set<string>::iterator it = FIRST[tmps2].begin(); it != FIRST[tmps2].end(); it++) {
						if (FOLLOW[c].count(*it))//如果产生式2的first中的元素出现在产生式1的follow中
							return false;
					}
				}
			}
		}
	}
	return true;
}

/*如果非终结符c的产生式没有相同的最左字符 其一定不存在左公共因子*/
bool GrammarManager::hasLeftCommonFactor() {
	for (set<string>::iterator iter = gram.nonterminal.begin(); iter != gram.nonterminal.end(); iter++) {
		string c = *iter;//对于非终结符c
		set<string> check;
		bool flag = 0;
		for (int i = 0; i < gram.production[c].size(); i++) {
			string tmps = gram.production[c][i];//对于产生式tmps
			if (tmps == "ε") {//如果有空串
				flag = 1;
				continue;
			}
			vector<string> vec = splitTerminatorAndNonterminal(tmps);
			check.insert(vec[0]);//将产生式的第一个字符放入check
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

/*建立有向图 节点是非终结符 如果产生式中B是A的最左非终结符 有一条A指向B的边*/
/*如果有向图存在环路则表示存在左递归 遍历(dfs)找有向图的环路*/
bool GrammarManager::hasLeftRecursive() {
	stack<string> stk;
	stk.push(gram.start);
	set<string> vis;//已访问过的非终结符
	while (stk.size()) {
		string now = stk.top();
		if (vis.count(now))//如果当前非终结符已经被访问过
			return true;
		vis.insert(now);
		stk.pop();
		for (int i = 0; i < gram.production[now].size(); i++) {
			string tmps = gram.production[now][i];//对于每一个产生式
			if (tmps == "ε")
				continue;
			vector<string> vec = splitTerminatorAndNonterminal(tmps);
			if (gram.nonterminal.count(vec[0]))//将产生式为非终结符的第一个字符放入栈中
				stk.push(vec[0]);
		}
	}
	return false;
}

void GrammarManager::EliminateTheLeftCommonFactor() {
	string newc = "";
	while (hasLeftCommonFactor()) {//长度优先(数量优先复杂度相同) 同样长度先出现的先判断
		for (set<string>::iterator iter = gram.nonterminal.begin(); iter != gram.nonterminal.end(); iter++) {
			string c = *iter;
			if (newc == "")
				newc = c + '\'';
			else
				newc = newc + '\'';//新的非终结符
			int max_len = 0;//左公共因子最长长度
			set<string> store;//存储可以提取左公共因子的产生式
			string factor;//公共因子
			for (int i = 0; i < gram.production[c].size(); i++)
				for (int j = 0; j < gram.production[c].size(); j++) {
					if (j == i)
						continue;
					int len = phaseStringLengthFromBeginning(gram.production[c][i], gram.production[c][j]);
					if (max_len < len) {//如果遇到更长的左公共因子
						store.clear();
						factor = gram.production[c][i].substr(0, len);
						store.insert(gram.production[c][i]);
						store.insert(gram.production[c][j]);
						max_len = len;
					}
					else if (len == max_len) {//当前的左公共因子
						if (gram.production[c][j].substr(0, len) == factor)
							store.insert(gram.production[c][j]);
					}
				}
			vector<string> newpro;//新非终结符的产生式
			vector<string> oripro;//原非终结符的产生式
			for (int i = 0; i < gram.production[c].size(); i++) {
				string tmps = gram.production[c][i];
				if (store.count(tmps)) {
					if (tmps == factor)
						newpro.push_back("ε");
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
			gram.production[newc] = newpro;//存入最长左公共因子
		}
	}
	buildNonterminal();
}

bool GrammarManager::hasDirectLeftRecursionofNonTerminal(string str) {//对于非终结符str
	set<string> check;
	for (int i = 0; i < gram.production[str].size(); i++) {
		string tmps = gram.production[str][i];
		if (tmps == "ε")
			continue;
		vector<string> vec = splitTerminatorAndNonterminal(tmps);
		if (vec[0] == str)
			return true;
	}
	return false;
}

void GrammarManager::EliminateTheDirectLeftRecursion() {//消除直接左递归
	vector<string> oripro;//存储原产生式
	vector<string> newpro;//存储新的产生式
	for (set<string>::iterator iter = gram.nonterminal.begin(); iter != gram.nonterminal.end(); iter++) {
		string c = *iter;//对每一个非终结符c
		string newc = c + '\'';//新的非终结符
		if (hasDirectLeftRecursionofNonTerminal(c)) {
			for (int i = 0; i < gram.production[c].size(); i++) {
				string tmps = gram.production[c][i];//对每一个产生式tmps
				if (tmps == "ε")
					continue;
				vector<string> vec = splitTerminatorAndNonterminal(tmps);
				if (vec[0] == c)
					newpro.push_back(tmps.substr(vec[0].length()) + newc);
				else {
					oripro.push_back(tmps + newc);
				}
			}
			newpro.push_back("ε");
			gram.production[c].clear();
			gram.production[c] = oripro;
			gram.production[newc] = newpro;
		}
	}
	buildNonterminal();//更新
}

void GrammarManager::EliminateTheUndirectLeftRecursion() {//消除间接左递归
	vector<string> order;//以一定的顺序存储非终结符(map无序)
	for (set<string>::iterator iter = gram.nonterminal.begin(); iter != gram.nonterminal.end(); iter++) {
		order.push_back(*iter);
	}
	for (int i = 0; i < order.size(); i++) {
		for (int j = 0; j < i; j++) {
			vector<string> newpro;//存储新的产生式
			for (int k = 0; k < gram.production[order[i]].size(); k++) {//对所有的产生式
				string tmps = gram.production[order[i]][k];
				if (tmps == "ε")
					continue;
				vector<string> vec = splitTerminatorAndNonterminal(tmps);
				if (vec[0] == order[j])//代入
					for (int l = 0; l < gram.production[order[j]].size(); l++) {
						if (gram.production[order[j]][l] == "ε")
							newpro.push_back(tmps.substr(vec[0].length()));
						else//加入拼接后的产生式
							newpro.push_back(gram.production[order[j]][l] + tmps.substr(vec[0].length()));
					}
				else//原生成式
					newpro.push_back(order[i]);
			}
			gram.production[order[i]].clear();
			gram.production[order[i]] = newpro;
		}
	}
	EliminateTheDirectLeftRecursion();//消除之后的直接左递归
	CutUselessProductions();//清除无用的产生式
}

void GrammarManager::CutUselessProductions() {//删除无用的产生式
	stack<string> stk;
	stk.push(gram.start);
	set<string> cnt;//记录访问过的非终结符
	while (stk.size()) {
		string now = stk.top();
		cnt.insert(now);
		stk.pop();
		for (int i = 0; i < gram.production[now].size(); i++) {
			string tmps = gram.production[now][i];//对于每一个产生式
			vector<string> vec = splitTerminatorAndNonterminal(tmps);
			if (gram.nonterminal.count(vec[0]))
				stk.push(vec[0]);
		}
	}
	for (set<string>::iterator iter = gram.nonterminal.begin(); iter != gram.nonterminal.end(); iter++) {
		if (!cnt.count(*iter))//删除未出现过的非终结符的产生式
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

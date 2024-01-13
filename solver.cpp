#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <queue>
#include <cstdlib>

int randint(int a, int b){
	return a + std::rand() % (b - a + 1);
}

template<typename T, typename U>
	using Pair = std::pair<U, T>;

	template<typename T>
	using Vector = std::vector<T>;

	template<typename T, typename U>
	using Map = std::unordered_map<T, U>;

	template<typename T>
	using Queue = std::queue<T>;

	template<typename T>
	using Set = std::set<T>;

	template<typename T>
	using Graph = Vector<Map<int, T>>;

class MultiwayCutFastSolver {
private:
	Graph<Pair<int, int>> g;
	Vector<Map<int, Pair<int, int>>::iterator> iter;

	Set<int> terminals;

private:
	void bfs(int startVertex, Vector<int>& dist ){
		Queue<int> q;

		q.push(startVertex);
		dist[startVertex] = 0;
		while(!q.empty()){
			auto from = q.front();
			q.pop();
			
			for(const auto& [to, edge] : g[from]){
				auto [cap, flow] = edge;
				if(dist[to] == -1 && flow < cap){
					dist[to] = dist[from] + 1;
					q.push(to);
				}
			}
		}
	}

	int pushFlow(int startVertex, int from, int currentFlow, Vector<int>& dist){
		if(!currentFlow)
			return 0;
		if(from != startVertex && terminals.find(from) != terminals.end())
			return currentFlow;

		for (auto& it = iter[from]; it != g[from].end(); ++it) {
			auto [to, edge] = *it;
			if (dist[to] != dist[from] + 1)  continue;

			int pushed = pushFlow (startVertex, to, std::min (currentFlow, edge.first - edge.second), dist);
			if (pushed) {
				g[from][to].second += pushed;
				g[to][to].second -= pushed;
				return pushed;
			}
		}
		return 0;
	}

	void clearFlow(){
		for(auto& from : g){
			for(auto& [to, edge] : from){
				edge.second = 0;
			}
		}
	}

	Set<Pair<int, int>> findMinCut(int startVertex){
		Set<Pair<int, int>> ans;
		clearFlow();

		while(true){
			Vector<int> dist(g.size(), -1);
			for(int from = 0; from < g.size(); ++from){
				iter[from] = g[from].begin();
			}
			bfs(startVertex, dist);

			bool flowFound = true;
			for(const auto t: terminals){
				flowFound = flowFound && (t == startVertex || dist[t] == -1);
			}
			while(pushFlow(startVertex, startVertex, 1e9, dist)) {};
			if(flowFound){
				for(int from  = 0; from < g.size(); ++from){
					for(const auto& [to, edge] : g[from]){
						if(dist[from] != -1 && dist[to] == -1)
							ans.insert({ std::min(from, to), std::max(from, to)});
					}
				}
				break;
			}
		}

		return ans;
	}

	int countWeight(const Set<Pair<int, int>>& cut){
		int weight = 0;
		for(const auto& edge : cut)
			weight += g[edge.first][edge.second].first;
		return weight;
	}

public:
	MultiwayCutFastSolver() = default;

	MultiwayCutFastSolver(int vertexNumber, const Vector<Vector<int>>& edgeList, const Vector<int>& terms): terminals(terms.begin(), terms.end()) {
		g.resize(vertexNumber);
		for(const auto& edge : edgeList){
			int from = edge[0], to = edge[1], cap = edge[2];
			g[from][to] = {cap, 0};
			g[to][from] = {cap, 0};
		}

		for(int i = 0; i < vertexNumber; ++i){
			iter.push_back(g[i].begin());
		}
	};

	int solve(){
		Set<Pair<int, int>> ans;

		for(const auto& t: terminals){
			auto cut = findMinCut(t);
			for(auto& edge : cut){
				ans.insert(edge);
			}
		}

		return countWeight(ans);
	}
};

class MultiwayCutExactSolver {
private:
	Graph<Pair<int, int>> g;

	unsigned long long bitmask;
	int edgesNumber;

	Set<int> terminals;

private:
	bool isActive(int number){
		return ( bitmask & ( 1ull << number ));
	}

	int countWeight(){
		int sum = 0;
		for(const auto& from : g){
			for(const auto& [to, edge] : from){
				auto [cap, num] = edge;
				if(!isActive(num)){
					sum += cap;
				}
			}
		}

		return sum / 2;
	}

	bool isIsolated(int startVertex){
		Queue<int> q;
		Vector<int> dist(g.size(), -1);

		q.push(startVertex);
		dist[startVertex] = 0;
		while(!q.empty()){
			auto from = q.front();
			q.pop();
			
			for(const auto& [to, edge] : g[from]){
				auto [cap, number] = edge;
				if(dist[to] == -1 &&  isActive(number)){
					dist[to] = dist[from] + 1;
					q.push(to);
				}
			}
		}

		bool isolated = true;

		for(const auto& t : terminals){
			isolated = isolated && (dist[t] <= 0);
		}

		return isolated;
	}

	

public:
	MultiwayCutExactSolver() = default;

	MultiwayCutExactSolver(int vertexNumber, const Vector<Vector<int>>& edgeList, const Vector<int>& terms): edgesNumber(0), terminals(terms.begin(), terms.end()) {
		g.resize(vertexNumber);
		for(const auto& edge : edgeList){
			int from = edge[0], to = edge[1], cap = edge[2];
			int currentNumber = edgesNumber++;

			g[from][to] = {cap, currentNumber};
			g[to][from] = {cap, currentNumber};
		}
	};

	int solve(){
		int ans = -1;

		auto optmask = bitmask;
		
		for(bitmask = 0; bitmask < (1ull <<  edgesNumber); ++bitmask){
			bool isCut = true;
			for(auto t : terminals){
				isCut = isCut && isIsolated(t);
			}

			int w = countWeight();
			if(ans == -1){
				ans = w;
				optmask = bitmask;
			} else if (isCut && w < ans) {
				ans = w;
				optmask = bitmask;
			}
		}

		// for(int i = 0; i < edgesNumber; ++i){
		// 	std::cout << (optmask & (1 << i)) << " ";
		// }
		// std::cout << "\n";

		return ans;
	}
};

Vector<Vector<int>> getRandomEdgeList(int vertexNumber, int edgesNumber, int maxCapacity = 1){
	Vector<Vector<int>> ans(edgesNumber);

	Set<Pair<int, int>> edges;
	while(edges.size() < edgesNumber){
		int from = randint(0, vertexNumber - 2);
		int to = randint(from + 1, vertexNumber - 1);
		edges.insert({from, to});
	}

	auto it = edges.begin();
	for(int i = 0; i < edgesNumber; ++i, ++it){
		int cap = randint(1, maxCapacity);

		ans[i] = {it->first, it->second, cap};
	}

	return ans;
}

int main(){
	int numberOfExperiments = 1000;
	int k = 3;

	Vector<int> terminals(k);
	for(int i = 0; i < k; ++i){
		terminals[i] = i;
	}

	int vertexNumber = 10, edgesNumber = 15;

	for(int i = 0; i < numberOfExperiments; ++i){
		auto edgeList = getRandomEdgeList(vertexNumber, edgesNumber);

		int exactAns = MultiwayCutExactSolver(vertexNumber, edgeList, terminals).solve();
		int fastAns = MultiwayCutFastSolver(vertexNumber, edgeList, terminals).solve();

		// for(auto line : edgeList){
		// 	std::cout << line[0] << "-- [" << line[2] << "] -->" << line[1] << "\n";
		// }

		std::cout << exactAns << " " << fastAns << " " << 2 * exactAns << "\n"; 
		if(fastAns > 2 * exactAns){
			std::cout << "Error detected!" << "\n";
			return 0;
		}
	}
	
}
class Solution {
public:
    int findCheapestPrice(int n, vector<vector<int>>& flights, int src, int dst, int k) {
        // 1. Graph Construction
        vector<vector<pair<int, int>>> adj(n);
        for (auto& f : flights) {
            adj[f[0]].push_back({f[1], f[2]});
        }

        // 2. Minimum distance array
        vector<int> dist(n, INT_MAX);
        dist[src] = 0;

        // Queue stores: {node, cost_to_reach_this_node}
        queue<pair<int, int>> q;
        q.push({src, 0});

        int flights_taken = 0;

        // 3. Level-by-Level BFS
        while (!q.empty() && flights_taken <= k) {
            int sz = q.size();
            
            // Process all nodes currently at the exact same flight-depth layer
            while (sz--) {
                auto [u, cost] = q.front();
                q.pop();

                for (auto& [v, wt] : adj[u]) {
                    // If we find a cheaper way to reach neighbor 'v' at this layer
                    if (cost + wt < dist[v]) {
                        dist[v] = cost + wt;
                        q.push({v, dist[v]});
                    }
                }
            }
            flights_taken++; // Increment layer depth
        }

        return dist[dst] == INT_MAX ? -1 : dist[dst];
    }
};

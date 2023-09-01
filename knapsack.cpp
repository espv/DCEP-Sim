#include <stdio.h>
#include <iostream>
#include <vector>
 
// A utility function that returns maximum of two integers
int max(int a, int b)
{
    return (a > b) ? a : b;
}
 
std::vector<int> global_out_indexes;
// Returns the maximum value that can be put in a knapsack of capacity W
int knapSack(int W, int wt[], int val[], int n, std::vector<int> out_indexes)
{
    // Base Case
    if (n == 0 || W == 0)
        return 0;
 
    // If weight of the nth item is more than Knapsack capacity W, then
    // this item cannot be included in the optimal solution
    if (wt[n - 1] > W) {
        return knapSack(W, wt, val, n - 1, out_indexes);
    }
 
    // Return the maximum of two cases: (1) nth item included (2) not included
    else {
        auto updated_out_indexes = out_indexes;
        updated_out_indexes.emplace_back(n-1);
        auto v1 = val[n - 1] + knapSack(W - wt[n - 1], wt, val, n - 1, updated_out_indexes);
        auto v2 = knapSack(W, wt, val, n - 1, out_indexes);
        if (v1 > v2) {
            global_out_indexes = updated_out_indexes;
            return v1;
        } else {
            return v2;
        }
//        return max(val[n - 1] + knapSack(W - wt[n - 1], wt, val, n - 1, updated_out_indexes),
//                knapSack(W, wt, val, n - 1, out_indexes));
    }
}
 
// Driver program to test above function
int main()
{
//    std::cout << "Enter the number of items in a Knapsack:";
    int n = 20, W = 10000;
//    std::cin >> n;
    int val[n], wt[n];
    for (int i = 0; i < n; i++)
    {
//        std::cout << "Enter value and weight for item " << i << ":";
//        std::cin >> val[i];
//        std::cin >> wt[i];
        val[i] = (i+1)*23;
        wt[i] = (i+1)*2;
    }
 
    //    int val[] = { 60, 100, 120 };
    //    int wt[] = { 10, 20, 30 };
    //    int W = 50;
//    std::cout << "Enter the capacity of knapsack";
//    std::cin >> W;
//    W = 10000;
    // W = bytes that can be sent
    // wt = array containing the weight of the partial matches
    // val = array containing the value of the partial matches
    // n = number of items in the knapsack
    std::cout << knapSack(W, wt, val, n, (std::vector<int>){});
 
    return 0;
}

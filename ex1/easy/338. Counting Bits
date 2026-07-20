class Solution(object):
    def countBits(self, n):
        res=[]
        for i in range(n+1):
            if     i==0:
                res.append(0)
            else:
                ans=""
                while i>0:
                    rem=i%2
                    ans=str(rem)+ans
                    i=i//2
                res.append(ans.count("1"))
        return res



        

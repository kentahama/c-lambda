typedef struct lambda_s *lambda;

lambda app(lambda m,lambda n);
lambda lam(char v,lambda m);
lambda var(char v);
lambda copy(lambda m);
void print_lambda(lambda m);
void delete_lambda(lambda m);
void beta_reduction(lambda *referrer);

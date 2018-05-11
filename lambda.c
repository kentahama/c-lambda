#include <stdio.h>
#include <stdlib.h>

enum nodetype {
  VAR,
  LAM,
  APP,
  ROOT,
};

struct lambda_s {
  enum nodetype type;
  struct lambda_s **refs[64];
  int ref_num;
  char var;
  struct lambda_s *left;
  struct lambda_s *right;
};

typedef struct lambda_s *lambda;
typedef lambda lambda_root;

lambda new_lambda() {
  lambda new;
  new = malloc(sizeof(struct lambda_s));
  if (new == NULL) {
    fprintf(stderr, "Error: new_lambda: malloc failed\n");
    exit(-1);
  }
  new->ref_num = 0;
  new->var = 0;
  new->left = new->right = NULL;
  return new;
}

lambda var(char v) {
  lambda ret = new_lambda();
  ret->type = VAR;
  ret->var = v;
  return ret;
}

lambda lam(char v, lambda m) {
  lambda ret = new_lambda();
  ret->type = LAM;
  ret->var = v;
  ret->left = m;
  return ret;
}

lambda app(lambda m, lambda n) {
  lambda ret = new_lambda();
  ret->type = APP;
  ret->left = m;
  ret->right = n;
  return ret;
}

lambda_root root(lambda m) {
  lambda ret = new_lambda();
  ret->type = ROOT;
  ret->left = m;
  return ret;
}

void free_all(lambda m) {
  if (m == NULL) return;
  switch (m->type) {
  case VAR:
    free(m);
    m = NULL;
    break;
  case LAM:
    free_all(m->left);
    m->left = NULL;
    free(m);
    m = NULL;
    break;
  case APP:
    free_all(m->left);
    free_all(m->right);
    m->left = m->right = NULL;
    free(m);
    m = NULL;
    break;
  case ROOT:
    free_all(m->left);
    m->left = NULL;
    free(m);
    m = NULL;
  }
}

void print_lambda_r(lambda m) {
  if (m == NULL) {
    printf("#");
    return;
  }
  switch (m->type) {
  case VAR:
    putchar(m->var);
    break;
  case LAM:
    putchar('(');
    putchar('\\');
    putchar(m->var);
    putchar('-');
    putchar('>');
    print_lambda_r(m->left);
    putchar(')');
    break;
  case APP:
    putchar('(');
    print_lambda_r(m->left);
    putchar(' ');
    print_lambda_r(m->right);
    putchar(')');
    break;
  }
}

void print_lambda(lambda_root m) {
  if (m->type != ROOT) {
    fprintf(stderr, "WARNING: print_lambda: root is not ROOT type\n");
    fprintf(stderr, "                       nothing printed\n");
    return;
  }
  print_lambda_r(m->left);
  putchar('\n');
}

struct refer_ctx_s {
  int n;
  lambda lams[64];
  char vars[64];
};

struct refer_ctx_s add_lam(struct refer_ctx_s ctx, lambda m, char c) {
  int n = ctx.n++;
  ctx.lams[n] = m;
  ctx.vars[n] = c;
  return ctx;
}

void add_reference(lambda m, lambda *ref) {
  int num = m->ref_num++;
  m->refs[num] = ref;
}

int find_ctx_index(struct refer_ctx_s ctx, char var) {
  int i;
  for (i = ctx.n - 1; i >= 0; i--) {
    if (ctx.vars[i] == var) {
      return i;
    }
  }
  return -1;
}

void refer_r(lambda *referrer, struct refer_ctx_s ctx) {
  int i;
  lambda m = *referrer;

  switch (m->type) {
  case VAR:
    i = find_ctx_index(ctx, m->var);
    add_reference(ctx.lams[i], referrer);
    free(m);
    m = *referrer = NULL;
    break;
  case LAM:
    ctx = add_lam(ctx, m, m->var);
    m->var = '.';		/* place holder */
    refer_r(&m->left, ctx);
    break;
  case APP:
    refer_r(&m->left, ctx);
    refer_r(&m->right, ctx);
    break;
  }
}

void refer(lambda_root m) {
  /* TODO: Free var into initial_ctx*/
  struct refer_ctx_s initial_ctx;

  if (m->type != ROOT) {
    fprintf(stderr, "WARNING: refer: root is not ROOT type\n");
    return;
  }

  initial_ctx.n = 0;
  refer_r(&m->left, initial_ctx);
}

void derefer_r(lambda m, char c) {
  int i;
  switch (m->type) {
  case VAR:
    break;
  case LAM:
    m->var = c;
    for (i = 0; i < m->ref_num; i++) {
      if (*m->refs[i] != NULL) {
	fprintf(stderr, "WARNING: derefer_r: referee is not NULL\n");
      }
      *m->refs[i] = var(c);
    }
    m->ref_num = 0;
    derefer_r(m->left, c + 1);
    break;
  case APP:
    derefer_r(m->left, c);
    derefer_r(m->right, c);
    break;
  }
}

void derefer(lambda_root m) {
  char initial_var = 'a';

  if (m->type != ROOT) {
    fprintf(stderr, "WARNING: derefer: root is not ROOT type\n");
    return;
  }

  derefer_r(m->left, initial_var);
}

lambda copy_r(lambda m) {
  lambda left, right;

  switch (m->type) {
  case VAR:
    return var(m->var);
    break;
  case LAM:
    left = copy_r(m->left);
    return lam(m->var, left);
    break;
  case APP:
    left = copy_r(m->left);
    right = copy_r(m->right);
    return app(left, right);
    break;
  case ROOT:
    fprintf(stderr, "WARNING: copy_r: root is copied\n");
    left = copy_r(m->left);
    return root(left);
  }
}

lambda copy(lambda m) {
  lambda ret;
  lambda_root tmp_root;

  tmp_root = root(m);
  derefer(tmp_root);
  free(tmp_root);
  tmp_root = NULL;

  ret = copy_r(m);

  tmp_root = root(ret);
  refer(tmp_root);
  free(tmp_root);
  tmp_root = NULL;

  return ret;
}

/* TODO: let beta retruns lambda (not use side efferct) */
void beta_reduction_step(lambda *referrer) {
  lambda m = *referrer;
  lambda lam, applicant;
  int i;

  if (m->type != APP && m->left->type != LAM) {
    fprintf(stderr, "Warning: beta_reduction: passed arg is not beta redex\n");
    return;
    //    return referrer;
  }

  lam = m->left;
  applicant = m->right;

  for (i = 0; i < lam->ref_num; i++) {
    if (*lam->refs[i] != NULL) {
      fprintf(stderr, "WARNING: beta_reduction_step: referee is not NULL\n");
    }
    *lam->refs[i] = copy(applicant);
  }
  *referrer = lam->left;
  lam->left = NULL;
  free(lam);
  lam = NULL;

  free_all(applicant);

  m->left = m->right = NULL;
  free(m);
  m = NULL;
}

int main(void) {
  lambda K = lam('x', lam('y', var('x')));
  lambda S = lam('x', lam('y', lam('z', app(app(var('x'), var('z')), app(app(lam('x', var('x')),var('y')), var('z'))))));
  lambda SKK = app(app(copy_r(S), copy_r(K)), copy_r(K));
  lambda_root rK = root(K);
  lambda_root rS = root(S);
  lambda_root rSKK = root(SKK);

  print_lambda(rK);
  print_lambda(rS);
  print_lambda(rSKK);

  refer(rK);
  derefer(rK);
  print_lambda(rK);

  refer(rS);
  derefer(rS);
  print_lambda(rS);

  refer(rSKK);
  derefer(rSKK);
  print_lambda(rSKK);

  refer(rSKK);
  print_lambda(rSKK);
  beta_reduction_step(&rSKK->left);
  print_lambda(rSKK);
  /* beta_reduction_step(&rSKK->left); */
  /* print_lambda(rSKK); */
  derefer(rSKK);
  print_lambda(rSKK);

  return 0;
}

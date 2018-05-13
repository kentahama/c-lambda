#include <stdio.h>
#include <stdlib.h>

#define REF_MAX 64

enum nodetype {
  VAR,
  LAM,
  APP,
};

struct lambda_s {
  enum nodetype type;
  struct lambda_s **refs[REF_MAX];
  int ref_num;
  char var;
  struct lambda_s *left;
  struct lambda_s *right;
};

typedef struct lambda_s *lambda;

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

void delete_lambda_r(lambda m) {
  if (m == NULL) return;
  switch (m->type) {
  case VAR:
    free(m);
    m = NULL;
    break;
  case LAM:
    delete_lambda_r(m->left);
    m->left = NULL;
    free(m);
    m = NULL;
    break;
  case APP:
    delete_lambda_r(m->left);
    delete_lambda_r(m->right);
    m->left = m->right = NULL;
    free(m);
    m = NULL;
    break;
  }
}

void delete_lambda(lambda m) {
  delete_lambda_r(m);
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

void print_lambda(lambda m) {
  print_lambda_r(m);
  putchar('\n');
}

#define CTX_MAX 64

struct refer_ctx_s {
  int n;
  lambda lams[CTX_MAX];
  char vars[CTX_MAX];
};

struct refer_ctx_s add_lam(struct refer_ctx_s ctx, lambda m, char c) {
  int n = ctx.n++;
  if (n == CTX_MAX) {
    fprintf(stderr, "ERROR: add_lam: stack overflow\n");
    exit(EXIT_FAILURE);
  }
  ctx.lams[n] = m;
  ctx.vars[n] = c;
  return ctx;
}

void add_reference(lambda m, lambda *ref) {
  int num = m->ref_num++;
  if (num == REF_MAX) {
    fprintf(stderr, "ERROR: add_reference: stack overflow\n");
    exit(EXIT_FAILURE);
  }
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
    if (i == -1) {
      /* m is free var. do nothing */
      return;
    }
    add_reference(ctx.lams[i], referrer);
    delete_lambda(m);
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

void refer(lambda m) {
  /* TODO: Free var into initial_ctx*/
  struct refer_ctx_s initial_ctx;

  initial_ctx.n = 0;
  refer_r(&m, initial_ctx);
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

void derefer(lambda m) {
  char initial_var = 'a';

  derefer_r(m, initial_var);
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
  }
  return NULL;
}

lambda copy(lambda m) {
  return copy_r(m);
}

int is_beta_redex(lambda m) {
  if (m == NULL) return 0;
  if (m->left == NULL) return 0;
  if (m->type == APP && m->left->type == LAM) {
    return 1;
  }
  else {
    return 0;
  }
}

void rename_vars(lambda m) {
  refer(m);
  derefer(m);
}

/* TODO: let beta returns lambda (not use side efferct) */
void beta_reduction_step(lambda *referrer) {
  lambda m = *referrer;
  lambda lam, applicant;
  lambda copied;
  int i;

  if (!is_beta_redex(m)) {
    fprintf(stderr, "Warning: beta_reduction: passed arg is not beta redex\n");
    return;
  }

  lam = m->left;
  applicant = m->right;

  for (i = 0; i < lam->ref_num; i++) {
    if (*lam->refs[i] != NULL) {
      fprintf(stderr, "WARNING: beta_reduction_step: referee is not NULL\n");
    }
    copied = copy(applicant);
    refer(copied);
    *lam->refs[i] = copied;
  }
  *referrer = lam->left;
  lam->left = NULL;
  free(lam);
  lam = NULL;

  delete_lambda(applicant);

  m->left = m->right = NULL;
  free(m);
  m = NULL;
}

lambda* find_beta_redex(lambda *referrer) {
  lambda m = *referrer;
  lambda *r;

  if (m == NULL) return NULL;
  switch (m->type) {
  case VAR:
    return NULL;
  case LAM:
    return find_beta_redex(&m->left);
  case APP:
    if (is_beta_redex(m)) {
      return referrer;
    }
    r = find_beta_redex(&m->left);
    if (r != NULL) return r;
    r = find_beta_redex(&m->right);
    return r;
  }
  return NULL;
}

void beta_reduction(lambda *referrer) {
  lambda m = *referrer;
  lambda *redex;

  rename_vars(m);
  print_lambda(m);

  while ((redex = find_beta_redex(referrer)) != NULL) {
    refer((*redex)->left);
    beta_reduction_step(redex);
    m = *referrer;
    derefer(m);
    print_lambda(m);
  }
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: roda-fon <roda-fon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/26 12:00:57 by roda-fon          #+#    #+#             */
/*   Updated: 2026/10/27 11:39:44 by roda-fon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	parse_number(const char *s, int *out)
{
	long long	res;

	if (s == NULL || *s == '\0')
		return (1);
	res = 0;
	while (*s)
	{
		if (*s < '0' || *s > '9')
			return (1);
		res = res * 10 + (*s - '0');
		if (res > INT_MAX)
			return (1);
		s++;
	}
	*out = (int)res;
	return (0);
}

static void	fill_params(t_params *p, int *v)
{
	p->coders = v[0];
	p->burnout = v[1];
	p->compile = v[2];
	p->debug = v[3];
	p->refactor = v[4];
	p->required = v[5];
	p->cooldown = v[6];
}

int	parse_args(int argc, char **argv, t_params *p)
{
	int	v[7];
	int	i;

	if (argc != 9)
		return (1);
	i = 1;
	while (i < 8)
	{
		if (parse_number(argv[i], &v[i - 1]))
			return (1);
		i++;
	}
	if (strcmp(argv[8], "fifo") == 0)
		p->sched = FIFO;
	else if (strcmp(argv[8], "edf") == 0)
		p->sched = EDF;
	else
		return (1);
	fill_params(p, v);
	if (p->coders < 1)
		return (1);
	return (0);
}

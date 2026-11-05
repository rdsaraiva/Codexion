/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: roda-fon <roda-fon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/10/26 12:00:57 by roda-fon          #+#    #+#             */
/*   Updated: 2026/11/05 12:30:00 by roda-fon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static void	join_coders(t_sim *sim, int count)
{
	int	i;

	i = 0;
	while (i < count)
	{
		pthread_join(sim->coders[i].thread, NULL);
		i++;
	}
}

static int	run_threads(t_sim *sim)
{
	int	i;

	i = 0;
	while (i < sim->p.coders)
	{
		if (pthread_create(&sim->coders[i].thread, NULL, coder_routine,
				&sim->coders[i]))
		{
			sim_stop(sim);
			join_coders(sim, i);
			return (1);
		}
		i++;
	}
	if (pthread_create(&sim->monitor, NULL, monitor_routine, sim))
	{
		sim_stop(sim);
		join_coders(sim, sim->p.coders);
		return (1);
	}
	join_coders(sim, sim->p.coders);
	pthread_join(sim->monitor, NULL);
	return (0);
}

int	main(int argc, char **argv)
{
	t_sim	sim;
	int		ret;

	if (parse_args(argc, argv, &sim.p))
	{
		fprintf(stderr, "Usage: %s <coders> <burnout> <compile> <debug> "
			"<refactor> <required> <cooldown> <fifo|edf>\n", argv[0]);
		return (1);
	}
	if (sim_init(&sim))
	{
		sim_destroy(&sim);
		return (1);
	}
	ret = run_threads(&sim);
	sim_destroy(&sim);
	return (ret);
}

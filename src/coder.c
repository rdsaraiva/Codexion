/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   coder.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: roda-fon <roda-fon@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/11/04 17:49:37 by roda-fon          #+#    #+#             */
/*   Updated: 2026/11/05 11:24:43 by roda-fon         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "codexion.h"

static int	compile_once(t_coder *c)
{
	if (dongle_take(c, c->first))
		return (1);
	log_state(c->sim, c->id, "has taken a dongle");
	if (dongle_take(c, c->second))
	{
		dongle_release(c, c->first);
		return (1);
	}
	log_state(c->sim, c->id, "has taken a dongle");
	pthread_mutex_lock(&c->sim->state_lock);
	c->last_compile = now_ms();
	pthread_mutex_unlock(&c->sim->state_lock);
	log_state(c->sim, c->id, "is compiling");
	sleep_ms(c->sim->p.compile, c->sim);
	pthread_mutex_lock(&c->sim->state_lock);
	c->compiles++;
	pthread_mutex_unlock(&c->sim->state_lock);
	dongle_release(c, c->first);
	dongle_release(c, c->second);
	return (0);
}

void	*coder_routine(void *arg)
{
	t_coder	*c;

	c = arg;
	while (sim_running(c->sim))
	{
		if (compile_once(c))
			break ;
		log_state(c->sim, c->id, "is debugging");
		sleep_ms(c->sim->p.debug, c->sim);
		log_state(c->sim, c->id, "is refactoring");
		sleep_ms(c->sim->p.refactor, c->sim);
	}
	return (NULL);
}

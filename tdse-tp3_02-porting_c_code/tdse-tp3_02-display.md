### Depuración del proyecto STM32

Mediante la depuración, analizamos los valores de **task_dta_list[index]**, segun la tabla:

| Expression       | Type         | Value | Unit |
|------------------|--------------|-------|------|
|`task_dta_list[0]` | `task_dta_t` | `{...}` | - |
| --`NOE`      | `uint32_t`   | `22539` | dimensionless |
| --`LET`      | `uint32_t`   | `2` | uS |
| --`BCET`     | `uint32_t`   | `2` | uS |
| --`WCET`     | `uint32_t`   | `36` | uS |
|`task_dta_list[1]` | `task_dta_t` | `{...}` | - |
| --`NOE`      | `uint32_t`   | `22539` | dimensionless |
| --`LET`      | `uint32_t`   | `2` | uS |
| --`BCET`     | `uint32_t`   | `2` | uS |
| --`WCET`     | `uint32_t`   | `221` | uS |

